#ifndef CLIENT_H
#define CLIENT_H

int send_ls_request(int p, int b);
int send_cr_request(int p, int b, char *minutes_str, char *hours_str, char *daysofweek_str, int argc, char **argv);
int send_rm_request(int p, uint64_t taskid);
int send_info_request(int p, int b, uint64_t taskid);
int send_so_request(int p, int b, uint64_t taskid);
int send_se_request(int p, int b, uint64_t taskid);
int send_tm_request(int p);

#endif