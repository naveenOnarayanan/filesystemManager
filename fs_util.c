#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int dir_list_id = 0;
struct dir_list {
	int id;
	FSDIR * dir;
	struct mount_list * mount;
	struct dir_list * next;
	struct dir_list * prev;
};

struct mount_list{
	char * serverIPorHost;
	int serverPort;
	char * localFolder;
	struct mount_list * next;
	struct mount_list * prev;
};

int file_list = 0;
struct file_desc_list {
	int id;
	int fd;
	char * file_path;
	struct mount_list * mount;
	struct file_desc_list * next;
	struct file_desc_list * prev;
};

struct dir_list * dir_head, * dir_tail;
struct mount_list * mount_head, * mount_tail;
struct file_desc_list * fd_head, * fd_tail;

const int FILTER_BY_PATH = 0;
const int FILTER_BY_ID = 1;

int dir_id_in_use(int id) {
	struct dir_list * dir = dir_head;
	while (dir != NULL) {
		if (dir->id == id) {
			return 1;
		}
		dir = dir->next;
	}

	return 0;
}

int fd_id_in_use(int id) {
	struct file_desc_list * fd = fd_head;

	while (fd != NULL) {
		if (fd->id == id) {
			return 1;
		}
		fd = fd->next;
	}

	return 0;
}

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
		if ((id_type == FILTER_BY_ID && tmp->id == *(int *)id) 
				|| (id_type == FILTER_BY_PATH && strcmp(tmp->file_path, id))) {
			return tmp;
		}
		tmp = tmp->next;
	}

	return NULL;
}

struct dir_list * find_dir(FSDIR * id) {
	struct dir_list * tmp = dir_head;

	while (tmp != NULL) {
		if (tmp->id == *(int *)id) {
			return tmp;
		}
		tmp = tmp->next;
	}

	return NULL;
}

int remove_dir(FSDIR * dir) {
	struct dir_list * dir_obj = find_dir(dir);
	if (dir_obj == NULL) {
		return -1;
	}

	if (dir_obj->prev == NULL) {
		dir_head = dir_obj->next;
		if (dir_head != NULL) {
			dir_head->prev = NULL;
		}
	} else {
		struct dir_list * tmp = dir_obj->prev;

		tmp->next = dir_obj->next;
		if (tmp->next != NULL) {
			tmp->next->prev = tmp;
		} else {
			dir_tail = tmp;
		}
	}

	free(dir_obj->dir);
	free(dir_obj);

	return 0;
}

int add_dir(FSDIR * dir, struct mount_list * mount) {
	while (dir_id_in_use(dir_list_id) == 1) {
		dir_list_id++;
	}

	struct dir_list * dir_obj = malloc(sizeof(struct dir_list));
	dir_obj->id = dir_list_id;
	dir_obj->dir = dir;
	dir_obj->mount = mount;
	dir_obj->next = NULL;
	dir_obj->prev = NULL;

	if (dir_head == NULL) {
		dir_head = dir_obj;
		dir_tail = dir_obj;
	} else {
		dir_tail->next = dir_obj;
		dir_obj->prev = dir_tail;
		dir_tail = dir_tail->next;
	}

	return dir_obj->id;
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

    strcpy(mount->serverIPorHost, srvIpOrDomName);

    mount->serverPort = srvPort;

    size_t folderAliasLength = strlen(localFolderName) + 1;
    mount->localFolder = malloc(folderAliasLength * sizeof(char));

    strcpy(mount->localFolder, localFolderName);

    mount->next = NULL;
    mount->prev = NULL;

    if (mount_head == NULL) {
        mount_head = mount;
        mount_tail = mount;
    } else {
        mount_tail->next = mount;
        mount->prev = mount_tail;
        mount_tail = mount_tail->next;
    }
}

int remove_fd(const int fd) {
	struct file_desc_list * fd_obj = find_fd(&fd, FILTER_BY_ID);

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

int add_fd(struct mount_list * mount, int fd) {
	struct file_desc_list * fd_obj = malloc(sizeof(struct file_desc_list));

	while (fd_id_in_use(file_list) == 1) {
		file_list++;
	}

	fd_obj->id = file_list;
	fd_obj->fd = fd;
	fd_obj->mount = mount;
	fd_obj->next = NULL;
	fd_obj->prev = NULL;

	if (fd_head == NULL) {
		fd_head = fd_obj;
		fd_tail = fd_obj;
	} else {
		fd_tail->next = fd_obj;
		fd_obj->prev = fd_tail;
		fd_tail = fd_tail->next;
	}

	return fd_obj->id;
}

const char * get_relative_path(const char * path, struct mount_list * mount) {
	const char * tmp = path;
    tmp+= strlen(mount->localFolder);
    return tmp;
}

