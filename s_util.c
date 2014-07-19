#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct resource_queue {
    char * path;
    int fd;
    struct resource_queue * next;
    struct resource_queue * prev;
}

struct resource_queue * resource_head, * resource_tail;

int resource_in_use(const char * path) {
  struct resource_queue * tmp = resource_head;
  while (tmp != NULL) {
    if (strcmp(tmp->path, path) == 0) {
      return 1;
    }
    tmp = tmp->next;
  }
  return 0;
}

int remove_resource(const int fd) {
  struct resource_queue * tmp = resource_head;
  while (tmp != NULL) {
    if (tmp->fd == fd) {
      if (tmp->prev == NULL) {
        resource_head = tmp->next;
        resource_head->prev = NULL;
      } else {
        struct resource_queue * resource = tmp->prev;
        resource->next = tmp->next;

        if (resource->next != NULL) {
          tmp->next->prev = resource;
        } else {
          resource_tail = resource;
        }
      }

      free(tmp->path);
      free(tmp);

      return 1;
    }

    tmp = tmp->next;
  }
  return 0;
}

void add_resource(const char * path, const int fd){
  struct resource_queue * resource = malloc(sizeof(struct resource_queue));
  size_t path_length = strlen(path) + 1;
  resource->path = malloc(path_length * sizeof(char));
  resource->fd = fd;

  if (resource_head == NULL) {
    resource_head = resource;
    resource_tail = resource;
  } else {
    resource_tail->next = resource;
    resource->prev = resource_tail;
    resource_tail = resource_tail->next;
  }
}