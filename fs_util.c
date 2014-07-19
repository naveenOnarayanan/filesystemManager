#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

struct mount_list{
	char * serverIPorHost;
	int serverPort;
	char * localFolder;
	struct mount_list * next;
	struct mount_list * prev;
};

struct file_desc_list {
	int fd;
	char * file_path;
	struct mount_list * mount;
	struct file_desc_list * next;
	struct file_desc_list * prev;
};

struct mount_list * mount_head, * mount_tail;
struct file_desc_list * fd_head, * fd_tail;

const int FILE_PATH = 0;
const int FILE_DESCRIPTOR = 1;

struct mount_list * find_mount(const char * folderPath) {
	struct mount_list * tmp = mount_head;

	while (tmp != NULL) {
		size_t localFolderSize = strlen(tmp->localFolder);
		if (strncmp(tmp->localFolder, folderPath, localFolderSize) == 0) {
			return tmp;
		}
		tmp = tmp->next;
	}

	return NULL;
}

struct file_desc_list * find_fd(const void * id, int id_type) {
	struct file_desc_list * tmp = fd_head;

	while (tmp != NULL) {
		if ((id_type == 0 && tmp->fd == *(int *)id) 
				|| (id_type == 1 && strcmp(tmp->file_path, id))) {
			return tmp;
		}
		tmp = tmp->next;
	}

	return NULL;
}

int remove_mount(const char * localFolderName) {
	struct mount_list * mount = find_mount(localFolderName);
    if (mount == NULL) {
        return -1;
    }

    if (mount->prev == NULL) {
        mount_head = mount->next;
        if (mount_head != NULL) {
        	mount_head->prev = NULL;
        }
    } else {
        struct mount_list * tmp = mount->prev;

        tmp->next = mount->next;
        if (tmp->next != NULL) {
            tmp->next->prev = tmp;
        } else {
        	mount_tail = tmp;
        }
    }

    free(mount->serverIPorHost);
    free(mount->localFolder);
    free(mount);

    return 0;
}

void add_mount(const char * srvIpOrDomName, const int srvPort, const char * localFolderName) {
	struct mount_list * mount = malloc(sizeof(struct mount_list));

	size_t srvIpOrDomNameSize = strlen(srvIpOrDomName) + 1;
    mount->serverIPorHost = malloc(srvIpOrDomNameSize * sizeof(char));
    printf("Was able to allocate memory\n");
    strcpy(mount->serverIPorHost, srvIpOrDomName);

    printf("Was able to strcpy serverIpOrHost\n");
    printf("IP: %s\n", mount->serverIPorHost);

    mount->serverPort = srvPort;

    printf("Port: %d\n", mount->serverPort);

    size_t folderAliasLength = strlen(localFolderName) + 1;
    mount->localFolder = malloc(folderAliasLength * sizeof(char));

    strcpy(mount->localFolder, localFolderName);

    printf("mount_head: %p\n", mount_head);
    printf("mount_tail: %p\n", mount_tail);

    if (mount_head == NULL) {
        mount_head = mount;
        mount_tail = mount;
    } else {
        mount_tail->next = mount;
        mount->prev = mount_tail;
        mount_tail = mount_tail->next;
    }

    printf("mount_head: %p\n", mount_head);
    printf("mount_tail: %p\n", mount_tail);
    printf("Folder name: %s\n", mount->localFolder);
}

int remove_fd(const int fd) {
	struct file_desc_list * fd_obj = find_fd(&fd, FILE_DESCRIPTOR);

	if (fd_obj == NULL) {
		return -1;
	}

	if (fd_obj->prev == NULL) {
		fd_head = fd_obj->next;
		if (fd_head != NULL) {
			fd_head->prev = NULL;
		}
	} else {
		struct file_desc_list * tmp = fd_obj->prev;

        tmp->next = fd_obj->next;
        if (tmp->next != NULL) {
            tmp->next->prev = tmp;
        } else {
        	fd_tail = tmp;
        }
	}

	free(fd_obj);
	return 0;
}

void add_fd(struct mount_list * mount, int fd) {
	struct file_desc_list * fd_obj = malloc(sizeof(struct file_desc_list));
	fd_obj->fd = fd;
	fd_obj->mount = mount;

	if (fd_head == NULL) {
		fd_head = fd_obj;
		fd_tail = fd_obj;
	} else {
		fd_tail->next = fd_obj;
		fd_obj->prev = fd_tail;
		fd_tail = fd_tail->next;
	}
}