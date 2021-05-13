#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>

#define INF 987654321
using namespace std;
int network[101][101];
int node_num;
struct node{
	int dist[101];
	int from[101];
}nd[101];


void message_run(FILE *fp, FILE *msgfp) {
	char cmd[1001];
	while (fgets(cmd, sizeof(cmd), msgfp) != NULL) {
		int st_node, end_node, next_node;
		char* token = NULL;
		vector<int> temp_vec;
		sscanf(cmd, "%d %d", &st_node, &end_node);
		if (nd[st_node].dist[end_node] != INF) {
			fprintf(fp, "from %d to %d cost %d hops ", st_node, end_node, nd[st_node].dist[end_node]);
			next_node = st_node;
			while (next_node != end_node) {
				temp_vec.push_back(next_node);
				next_node = nd[next_node].from[end_node];
			}
			for (int i = 0; i < int(temp_vec.size()); i++) fprintf(fp, "%d ", temp_vec[i]);
			fprintf(fp, "message");
			token = strtok(cmd, " ");
			for (int i = 0; i < 2; i++) token = strtok(NULL, " ");
			while (token != NULL) {
				fprintf(fp, " %s", token);
				token = strtok(NULL, " ");
			}
		}
		else {
			fprintf(fp, "from %d to %d cost infinite hops unreachable message", st_node, end_node);
			token = strtok(cmd, " ");
			for (int i = 0; i < 2; i++) token = strtok(NULL, " ");
			while (token != NULL) {
				fprintf(fp, " %s", token);
				token = strtok(NULL, " ");
			}
		}
	}
	fprintf(fp, "\n");

}

void bellman_ford(FILE *fp) {
	int update_cnt = 0;
	for(int i = 0; i<node_num; i++) {
		for(int j = 0; j<node_num; j++) {
			if(i == j) nd[i].dist[j] = 0, nd[i].from[j] = i;
			else nd[i].dist[j] = INF;
		}
	}
	do{
		update_cnt = 0;
		for(int i = 0; i<node_num; i++) {
			for(int j = 0; j<node_num; j++) {
				//if(network[j][j] >= INF) continue;
				for(int k = 0; k<node_num; k++) {
					if(nd[i].dist[j] >= network[i][k] + nd[k].dist[j]) {
						if(nd[i].dist[j] == network[i][k] + nd[k].dist[j]) {
							update_cnt--;
							nd[i].from[j] = min(nd[i].from[j], k);
						}
						else {
							nd[i].from[j] = k;
						}
						nd[i].dist[j] = network[i][k] + nd[k].dist[j];
						//nd[i].from[j] = k;
						update_cnt++;
					}
				}
			}
		}
	}while(update_cnt!=0);

	for(int i = 0; i<node_num; i++) {
		for(int j = 0; j<node_num; j++)	{
			if(nd[i].dist[j] == INF) continue;
			fprintf(fp, "%d %d %d\n", j, nd[i].from[j], nd[i].dist[j]);
		}
		fprintf(fp,"\n");	
	}
	return;
}


int main(int argc, char *argv[]) {

	FILE *tofofp, *msgfp, *chgfp;
	FILE* outputfp;
	int st_node, end_node, dist;
	char changes[20];
	if(argc != 4){
		fprintf(stderr, "usage: linkstate topologyfile messagesfile changesfile\n");
		exit(1);
	}

	if((tofofp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Error: open input file\n");
		exit(1);
	}
	if((msgfp = fopen(argv[2], "r")) == NULL) {
		fprintf(stderr, "Error: open input file\n");
		exit(1);
	}
	if((chgfp = fopen(argv[3], "r")) == NULL) {
		fprintf(stderr, "Error: open input file\n");
		exit(1);
	}
	if((outputfp = fopen("output_dv.txt", "w")) == NULL) {
		fprintf(stderr, "Error: open input file\n");
		exit(1);
	}

	fscanf(tofofp, "%d", &node_num);
	for(int i = 0; i<node_num; i++) {
		for(int j = 0; j<node_num; j++) {
			network[i][j] = INF;
		}
	}

	while(feof(tofofp)==0){
		fscanf(tofofp, "%d %d %d", &st_node, &end_node, &dist);
		network[st_node][end_node] = dist;
		network[end_node][st_node] = dist;
		nd[st_node].dist[end_node] = dist;
		nd[end_node].dist[st_node] = dist;
		nd[st_node].from[end_node] = end_node;
		nd[end_node].from[st_node] = st_node;
	}
	bellman_ford(outputfp);
	message_run(outputfp, msgfp);
	fclose(msgfp);

	while(fgets(changes, sizeof(changes), chgfp) != NULL) {
		msgfp = fopen(argv[2], "r");
		sscanf(changes, "%d %d %d", &st_node, &end_node, &dist);
		int real_dist;
		if(dist == -999) real_dist = INF;
		else real_dist = dist;
		network[st_node][end_node] = real_dist;
		network[end_node][st_node] = real_dist;
		nd[st_node].dist[end_node] = real_dist;
		nd[end_node].dist[st_node] = real_dist;
		bellman_ford(outputfp);
		message_run(outputfp, msgfp);
		fclose(msgfp);
	}
	// format (dest, next. distance)
	printf("Complete. Output file written to output_dv.txt.\n");
	fclose(tofofp);
	fclose(chgfp);
	fclose(outputfp);
	return 0;
}
