#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  u32 remaining_time;
  u32 end_time;
  u32 start_execute_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

void visual(struct process_list list){
  struct process *np;
  TAILQ_FOREACH(np, &list, pointers){
    printf("%d\t", np->pid);
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;
  
  /* Your code here */
  u32 universal_time = 0;

  //initialized all remaining times
  for(int proc=0; proc<size; proc++){
    data[proc].remaining_time = data[proc].burst_time;
  }

  //add new processes at time 0
  for(int proc=0; proc<size; proc++){
    if(data[proc].arrival_time==0){
      // printf("Add initial P%d: \t%d\n", data[proc].pid, data[proc].remaining_time);
      //add to queue
      TAILQ_INSERT_TAIL(&list, &data[proc], pointers);
    }
  }
  // visual(list);

  bool finished = false;
  while(!finished){
    //pop 1st process from front
    struct process *first = TAILQ_FIRST(&list);
    if(first == NULL){
      continue;
    }
    // printf("Remove P%d: \t%d\n", first->pid, first->remaining_time);
    TAILQ_REMOVE(&list, first, pointers);
    // visual(list);
    //run process for min(quantum, remaining time)
    int min_time = (quantum_length < first->remaining_time) ? quantum_length : first->remaining_time;
    for(int t=0; t<min_time; t++){
      if(first->remaining_time == first->burst_time) first->start_execute_time = universal_time;
      first->remaining_time--;
      universal_time++;
      //add any new processes just arrived
      for(int proc=0; proc<size; proc++){
        if(data[proc].arrival_time==universal_time){
          // printf("Add P%d: \t%d\n", data[proc].pid, data[proc].remaining_time);
          // visual(list);
          TAILQ_INSERT_TAIL(&list, &data[proc], pointers);
        }
      }
    }
    //if remaining time > 0 add back in queue
    if(first->remaining_time>0){
      // printf("ReAdd P%d: \t%d\n", first->pid, first->remaining_time);
      TAILQ_INSERT_TAIL(&list, first, pointers);
      // visual(list);
    }
    else{
      first->end_time = universal_time;
      finished = true;
      struct process *np;
      TAILQ_FOREACH(np, &list, pointers){
        if(np->remaining_time>0){
          finished = false;
          break;
        }
      }
    }
  }
  //calculate times
  for(int proc=0; proc<size; proc++){
    total_waiting_time += data[proc].end_time - data[proc].arrival_time - data[proc].burst_time;
    total_response_time += data[proc].start_execute_time - data[proc].arrival_time;
  }
  
  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
