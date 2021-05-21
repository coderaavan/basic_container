#define _GNU_SOURCE
#include<sched.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/utsname.h>
#include<sys/mount.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define STACK_SIZE 1024*1024

static char child_stack[STACK_SIZE];

void errExit(char *msg){
    printf("%s failed\n", msg);
    exit(EXIT_FAILURE);
}

static void cgroup_setup(pid_t child_pid){
    char str_child_pid[10];
    sprintf(str_child_pid, "%d", child_pid);
    char str_pid_write[200] = "echo ";
    strcat(str_pid_write, str_child_pid);
    strcat(str_pid_write," > /sys/fs/cgroup/memory/pa3/tasks");
    
    //system("mkdir /sys/fs/cgroup/memory/pa3");
    mkdir("/sys/fs/cgroup/memory/pa3", S_IRWXU);
    system("echo \"50000000\" > /sys/fs/cgroup/memory/pa3/memory.limit_in_bytes");
    system("echo \"0\" > /sys/fs/cgroup/memory/pa3/memory.swappiness");
    system(str_pid_write);
}

static void netns_setup(char x[5], char y[5]){
    char cmd[200] = " ";

    strcpy(cmd,"ip link add veth");
    strcat(cmd,x);
    strcat(cmd," type veth peer name veth");
    strcat(cmd,y);
    system(cmd);

    strcpy(cmd,"ip netns add vnet");
    strcat(cmd,x);
    system(cmd);

    strcpy(cmd,"ip link set veth");
    strcat(cmd,x);
    strcat(cmd," netns vnet");
    strcat(cmd,x);
    system(cmd);

    strcpy(cmd,"ip -n vnet");
    strcat(cmd,x);
    strcat(cmd," addr add 10.0.");
    strcat(cmd,x);
    strcat(cmd,".0/24 dev veth");
    strcat(cmd,x);
    system(cmd);
    
    strcpy(cmd,"ip -n vnet");
    strcat(cmd,x);
    strcat(cmd," link set veth");
    strcat(cmd,x);
    strcat(cmd," up");
    system(cmd);
    
    strcpy(cmd,"ip -n vnet");
    strcat(cmd,x);
    strcat(cmd," link set lo up");
    system(cmd);
    
    strcpy(cmd,"ip addr add 10.0.");
    strcat(cmd,y);
    strcat(cmd,".0/24 dev veth");
    strcat(cmd,y);
    system(cmd);
    
    strcpy(cmd,"ip link set veth");
    strcat(cmd,y);
    strcat(cmd," up");
    system(cmd);
    
    strcpy(cmd,"ip -n vnet");
    strcat(cmd,x);
    strcat(cmd," route add 10.0.");
    strcat(cmd,y);
    strcat(cmd,".0/24 dev veth");
    strcat(cmd,x);
    system(cmd);
    
    strcpy(cmd,"ip route add 10.0.");
    strcat(cmd,x);
    strcat(cmd,".0/24 dev veth");
    strcat(cmd,y);
    system(cmd);
}

static int childFunc(void *arg){
    sleep(2);

    char **args = arg;
    struct utsname uts;

    printf("PID of child process within container is %ld\n\n",(long)getpid());

    if(uname(&uts) == -1)
        errExit("uname");
    printf("Hostname before changing hostname is %s\n\n", uts.nodename);

    if(sethostname(args[2],strlen(args[2])) == -1)
        errExit("sethostname");

    if(uname(&uts) == -1)
        errExit("uname");
    printf("Hostname after changing hostname is %s\n\n", uts.nodename);
   
    int n = atoi(args[3]);
    int x = 2*n-1, y = 2*n;
    char path[50] = "/var/run/netns/vnet";
    char x_str[5];
    sprintf(x_str, "%d", x);
    strcat(path,x_str);
    int fd = open(path, O_RDONLY);
    if(fd == -1)
       errExit("open in child");
    
    if(setns(fd, CLONE_NEWNET) == -1)
        errExit("setns");

    if(chdir(args[1])==-1)
        errExit("chdir");

    if(chroot(args[1])==-1)
        errExit("chroot");

    mount("proc", "/proc", "proc", 0, NULL);

    char *args_shell[2];
    args_shell[0] = "/bin/bash";
    args_shell[1] = NULL;
    execve(args_shell[0], args_shell, NULL);   
}

int main(int argc, char *argv[]){
    
    int flags = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNET | SIGCHLD;

    if(argc != 4){
        printf("Insufficient number of arguements.\n");
        printf("Please pass rootfs, hostname and container_index in that particular order\n");
        printf("Exiting...\n");
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[3]);
    int x = 2*n-1, y = 2*n;
    char path[50] = "/var/run/netns/vnet";
    char x_str[5], y_str[5];
    sprintf(x_str, "%d", x);
    sprintf(y_str, "%d", y);
    strcat(path,x_str);

    int fd = open(path, O_RDONLY);
    if(fd == -1){
        netns_setup(x_str, y_str); 
        fd = open(path, O_RDONLY);
        if(fd == -1)
            errExit("open in parent");
        else
            close(fd);
    }

    pid_t child_pid = clone(childFunc, child_stack + STACK_SIZE, flags, (void *)argv);

    if(child_pid == -1){
        errExit("clone");
    }

    cgroup_setup(child_pid);

    if(waitpid(child_pid, NULL, 0) == -1){
        errExit("waitpid");
    }

    exit(EXIT_SUCCESS);
}