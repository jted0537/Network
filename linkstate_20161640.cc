#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <queue>

#define INF 987654321
using namespace std;
vector<pair<int, int> > network[101];
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

void dijkstra(FILE *fp) {
	for(int i = 0; i<node_num; i++) {
		for(int j = 0; j<node_num; j++) {
            if(i == j) {
				nd[i].dist[j] = 0;
				nd[i].from[j] = i;
				continue;
			}
            nd[i].dist[j] = INF;
            nd[j].dist[i] = INF;
		}
	}

    for(int i = 0; i<node_num; i++) {
        int st_node = i;
        priority_queue<pair<int, int> > pq;
        pq.push(make_pair(0, st_node));
        while(!pq.empty()) {
            int cur_node = pq.top().second;
            int dist = -pq.top().first;
            pq.pop();
			for(int j = 0; j<(int)network[cur_node].size(); j++) {
				int next_node = network[cur_node][j].first;
				int next_dist = network[cur_node][j].second;

				if(nd[st_node].dist[next_node] > dist+next_dist) {
					nd[st_node].dist[next_node] = dist+next_dist;
					//nd[next_node].dist[st_node] = dist+next_dist;
					nd[next_node].from[st_node] = cur_node;
					pq.push(make_pair(-nd[st_node].dist[next_node], next_node));
				}
			}
        }
    }

    for(int i = 0; i<node_num; i++) {
		for(int j = 0; j<node_num; j++)	{
			printf("%d ", nd[i].dist[j]);
			if(nd[i].dist[j] == INF) continue;
			fprintf(fp, "%d %d %d\n", j, nd[i].from[j], nd[i].dist[j]);
		}
		printf("\n");
		fprintf(fp,"\n");	
	}
	printf("\n");
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
	if((outputfp = fopen("output_ls.txt", "w")) == NULL) {
		fprintf(stderr, "Error: open input file\n");
		exit(1);
	}

	fscanf(tofofp, "%d", &node_num);

	while(feof(tofofp)==0){
		fscanf(tofofp, "%d %d %d ", &st_node, &end_node, &dist);
		network[st_node].push_back(make_pair(end_node, dist));
		network[end_node].push_back(make_pair(st_node, dist));
	}
	dijkstra(outputfp);
	message_run(outputfp, msgfp);
	fclose(msgfp);
	while(fgets(changes, sizeof(changes), chgfp) != NULL) {
		bool flag1 = false, flag2 = false;
		msgfp = fopen(argv[2], "r");
		sscanf(changes, "%d %d %d", &st_node, &end_node, &dist);
		int real_dist;
		if(dist == -999) real_dist = INF;
		else real_dist = dist;

		for(int i = 0; i<(int)network[st_node].size(); i++) {
			if(network[st_node][i].first == end_node) {
				network[st_node][i].second = real_dist;
				flag1 = true;
			}
		}
		if(!flag1) network[st_node].push_back(make_pair(end_node, real_dist));
		
		for(int i = 0; i<(int)network[end_node].size(); i++) {
			if(network[end_node][i].first == st_node) {
				network[end_node][i].second = real_dist;
				flag2 = true;
			}
		}
		if(!flag2) network[end_node].push_back(make_pair(st_node, real_dist));
		for(int i = 0; i<network[3].size(); i++) {
			printf("%d %d      ", network[3][i].first, network[3][i].second);
		}
		printf("\n");
		dijkstra(outputfp);
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
