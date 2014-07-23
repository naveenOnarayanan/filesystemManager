#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

host_folder hostFolder;

int FILTER_BY_PATH = 0;
int FILTER_BY_ID = 1;

struct fsDirent {
    char entName[256];
    unsigned char entType; /* 0 for file, 1 for folder,
            -1 otherwise. */
};

int dir_size = 0;
struct dir_queue {
  int id;
  DIR * dir;
  struct dir_queue * next;
  struct dir_queue * prev;
};

struct client_queue {
  char * ip;
  unsigned short int port;
  struct client_queue * next;
};

struct resource_queue {
    char * path;
    int fd;
    int mode;
    struct client_queue * client;
    struct resource_queue * next;
    struct resource_queue * prev;
};

struct dir_queue * dir_head, * dir_tail;
struct client_queue * current_client;
struct resource_queue * resource_head, * resource_tail;

struct dir_queue * find_dir(int id) {
  struct dir_queue * tmp = dir_head;
  while (tmp != NULL) {
    if (tmp->id == id) {
      return tmp;
    }
    tmp = tmp->next;
  }

  return NULL;
}

// void dequeue_client(struct resource_queue * resource) {
//   struct client_queue * tmp = resource->client;
//   resource->client = resource->client->next;

//   free(tmp);
// }

// int add_client(struct resource_queue * resource) {
//   struct client_queue * tmp = 
// }

bool client_use_resource(struct resource_queue * resource, struct client_queue * client, int mode) {
  
  return resource == NULL
      || (resource != NULL && resource->client == NULL)
      || (resource->client != NULL 
          && strcmp(resource->client->ip, client->ip) 
          && resource->client->port == client->port)
      || (resource->client != NULL 
          && resource->mode == mode && mode == 0
  );
}

void add_client_to_resource(struct resource_queue * resource, struct client_queue * client) {
  struct client_queue * tmp = resource->client;

  while (tmp != NULL && tmp->next != NULL) {
    tmp = tmp->next;
  }

  struct client_queue * client_obj = (struct client_queue *) malloc(sizeof(struct client_queue));
  size_t ip_size = strlen(client->ip) + 1;
  client_obj->ip = malloc(ip_size * sizeof(char));
  strcpy(client_obj->ip, client->ip);
  client_obj->port = client->port;
  client_obj->next = NULL;

  if (tmp == NULL) {
    tmp = client_obj;
  } else {
    tmp->next = client_obj;
  }
}

void add_client(char * client_ip, unsigned short int client_port) {
  struct client_queue * client = (struct client_queue *) malloc(sizeof(struct client_queue));
  size_t client_ip_len = strlen(client_ip) + 1;
  client->ip = malloc(client_ip_len * sizeof(char));
  strcpy(client->ip, client_ip);
  client->port = client_port;
  client->next = NULL;

  current_client = client;
}

void remove_current_client() {
  if (current_client != NULL) {
    free(current_client->ip);
    free(current_client);
  }
}

struct resource_queue * find_resource(const void * query, int filter) {
  struct resource_queue * tmp = resource_head;
  while (tmp != NULL) {
    if ((filter == FILTER_BY_PATH && strcmp((char *)tmp->path, query) == 0) 
        || (filter == FILTER_BY_ID && tmp->fd == *(int *)query)) {
      return tmp;
    }
    tmp = tmp->next;
  }
  return NULL;
}

int remove_dir(const int id) {
  struct dir_queue * tmp = find_dir(id);
  if (tmp == NULL) {
    return -1;
  }

  if (tmp->prev == NULL) {
    dir_head = tmp->next;
    if (dir_head != NULL) {
      dir_head->prev = NULL;
    } else {
      dir_tail = NULL;
    }
  } else {
    struct dir_queue * dir_obj = tmp->prev;
    dir_obj->next = tmp->next;

    if (dir_obj->next != NULL) {
      dir_obj->next->prev = dir_obj;
    } else {
      dir_tail = dir_obj;
    }
  }

  free(tmp);

  return 0;
}

int dir_id_exists(int id) {
  struct dir_queue * dir = dir_head;
  while (dir != NULL) {
    if (dir->id == id) {
      return 1;
    }
    dir = dir->next;
  }

  return 0;
}

int add_dir(DIR * dir) {
  struct dir_queue * dir_obj = malloc(sizeof(struct dir_queue));

  while (dir_id_exists(dir_size) == 1) {
    dir_size++;
  }

  dir_obj->id = dir_size;
  dir_obj->dir = dir;
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

int remove_resource(const int fd) {
  struct resource_queue * tmp = find_resource(&fd, FILTER_BY_ID);

  if (tmp == NULL) {
    return 0;
  }

  if (tmp->client != NULL) {
    struct client_queue * client = tmp->client;
    tmp->client = tmp->client->next;



    free(client->ip);
    free(client);
  }

  if (tmp->prev == NULL) {
    resource_head = tmp->next;
    if (resource_head != NULL) {
      resource_head->prev = NULL;
    } else {
      resource_tail = NULL;
    }
  } else {
    struct resource_queue * resource = tmp->prev;
    resource->next = tmp->next;

    if (resource->next != NULL) {
      resource->next->prev = resource;
    } else {
      resource_tail = resource;
    }
  }

  free(tmp->path);
  free(tmp);

  return 1;
}

struct resource_queue * add_resource(const char * path, const int fd, const int mode, struct client_queue * client){
  struct resource_queue * resource = malloc(sizeof(struct resource_queue));
  size_t path_length = strlen(path) + 1;
  resource->path = malloc(path_length * sizeof(char));
  strcpy(resource->path, path);
  resource->mode = mode;
  resource->fd = fd;
  resource->next = NULL;
  resource->prev = NULL;
  resource->client = malloc(sizeof(struct resource_queue));
  size_t client_ip_len = strlen(client->ip) + 1;
  resource->client->ip = malloc(client_ip_len * sizeof(char));
  strcpy(resource->client->ip, client->ip);
  resource->client->port = client->port;
  resource->client->next = NULL;

  if (resource_head == NULL) {
    resource_head = resource;
    resource_tail = resource;
  } else {
    resource_tail->next = resource;
    resource->prev = resource_tail;
    resource_tail = resource_tail->next;
  }

  return resource;
}

char * append_local_path(char * folderName) {
  size_t totalLength = hostFolder.hostedFolderNameLength;
  totalLength += strlen(folderName) + 1;

  char * serverFolder = malloc(totalLength * sizeof(char));
  strcpy(serverFolder, (char *) hostFolder.hostedFolderName);
  strcat(serverFolder, (char *) folderName);

  return serverFolder;
}

int * get_error() {
  int * error = malloc(sizeof(int));
  * error = errno;
  return error;
}

int * set_error(int errorCode){
  errno = errorCode;
  return get_error();
}

