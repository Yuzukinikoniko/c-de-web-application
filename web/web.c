//構造体じゃなくてポインタのポインタを使ってみる
//ロックを考える方
//最終の方
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h> //exit
#include <string.h> //memset()
#include <stdio.h> //perror
#include <pthread.h>

#define BUFSIZE 2048 //適当
#define SIZE 2048 //適当
#define FILED_SIZE 1000 //適当

#define M 1000 //適当
#define N 1000 //適当

pthread_mutex_t mutex;

unsigned int getFileSize(const char *path) {
    int size, read_size;
    char read_buf[SIZE];
    FILE *f;

     f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }
    size = 0;
    do {
        read_size = fread(read_buf, 1, SIZE, f);
        size += read_size;
    } while(read_size != 0);

    fclose(f);
    return size;
}


void parseBody(char *body){
    char data_json[SIZE]; //jsonファイルから読み込む
    char buf[SIZE]; //jsonファイルに書き込むための配列
    char add_data[SIZE]; //新しくファイルに追加するための情報を格納する
    FILE *f;
    int file_size; //ファイルサイズを格納
    if(pthread_mutex_lock(&mutex)!=0){
        printf("ロックエラー\n");
    }
    file_size=getFileSize("data.json");
    //data.jsonからファイルを読み込み
    f = fopen("data.json", "r");
    fread(data_json, 1, file_size, f);  //freadでやる意味がわからんfreadってバイナリファイルを読む関数じゃないの？
    fclose(f);
    
    memcpy(buf,data_json,file_size-3); //いる部分だけ格納する
    sprintf(add_data,",\n\t%s\n]\n",body); //\t入れてもずれるんだけど、javascript的には問題ないのであんまり重要ではない

    memcpy(&buf[file_size-3],add_data,strlen(add_data));//buf3の長さを格納する

    //data.jsonに書き込む
    f = fopen("data.json","w");
    fputs(buf,f);
    fclose(f);
    if(pthread_mutex_unlock(&mutex)!=0){
        printf("アンロックエラー\n");
    }
}

int parseRequestMessage(char *method,char *target,char *request_filed[][M],char *request_message,char *request_body) {

    char *tmp_method;
    char *tmp_target;
    char *line;
    char *token; //区切りをする
    int i=0,j=1,k;
    int n,m;
    /* リクエストメッセージの１行目のみ取得 */
    line = strtok(request_message, "\n");
    printf("%s\n",line);
    //ヘッダーフィールド?を解析する
    token=strtok(NULL,"\n");
    i=0;
     //一行ずつフィールド部分を解析するが、今回は使わないのでどうでも良い
    while((strcmp(token,"\r"))!=0){
        request_filed[i][0]=token;
       token=strtok(NULL,"\n");
       i++;
    }
    token=strtok(NULL,"\n");
    if(token==NULL){
        request_body=token;
    }else{
         strcpy(request_body, token); 
    }
    k=i;
    for(i=0;i<k;i++){ //request_fild[k][0]には\rが入っている
        j=0;
        token=request_filed[i][0];
        request_filed[i][0]=strtok(token,":"); //request_filedに直接代入しているから最後にNULLが入って終わっちゃう(今回はどうでも良いけど)
        while(request_filed[i][j]!=NULL){ //一行ずつフィールド部分を解析する
            request_filed[i][j]=strtok(NULL,":");
            j++;
        }
    }
    tmp_method = strtok(line, " ");
    if (tmp_method == NULL) {
        printf("get method error\n");
        return -1;
    }
    strcpy(method, tmp_method);
    /* 次の" "までの文字列を取得しtargetにコピー */
    tmp_target = strtok(NULL, " ");
    if (tmp_target == NULL) {
        printf("get target error\n");
        return -1;
    }
    strcpy(target, tmp_target);

    return 0;
}


int getProcessing(char *body, char *file_path) {

    FILE *f;
    int file_size;
    /* ファイルサイズを取得 */
    file_size = getFileSize(file_path);
    if (file_size == 0) {
        /* ファイルサイズが0やファイルが存在しない場合は404を返す */
        file_size = getFileSize("index404.html");
        f = fopen("index404.html", "r");
        fread(body, 1, file_size, f);
        fclose(f);
        return 404;
    }else{
        f = fopen(file_path, "r"); //なんで"r"で開いてfreadなの?
        fread(body, 1, file_size, f);
        fclose(f);
        return 200;
    }
    /* ファイルを読み込んでボディとする */
}


int createResponseMessage(char *response_message, int status, char *header, char *body, unsigned int body_size) {
    unsigned int no_body_len;
    unsigned int body_len;

    response_message[0] = '\0';

    if (status == 200) {
        /* レスポンス行とヘッダーフィールドの文字列を作成 */
        sprintf(response_message, "HTTP/1.1 200 OK\r\n%s\r\n", header);
        printf("HTTP/1.1 200 OK\n");
        no_body_len = strlen(response_message);
        body_len = body_size;

        /* ヘッダーフィールドの後ろにボディをコピー */
        memcpy(&response_message[no_body_len], body, body_len);
    } else if (status == 404) {
        /* レスポンス行とヘッダーフィールドの文字列を作成 */
        sprintf(response_message, "HTTP/1.1 404 Not Found\r\n%s\r\n", header);
        printf("HTTP/1.1 404 Not Found\n");
        no_body_len = strlen(response_message);
        body_len = body_size;
        //body_len = 0;
        memcpy(&response_message[no_body_len], body, body_len);
        
    } else {
        /* statusが200・404以外はこのプログラムで非サポート */
        printf("Not support status(%d)\n", status);
        return -1;
    }
    //printf("responses%sここまで\n",response_message);
    return no_body_len + body_len;
}
void* pthread_socket(void *sock){
    int psock = *(int*)sock;
    int request_size, response_size;
    char request_message[SIZE];
    char response_message[SIZE];
    char header_field[SIZE];
    char body[SIZE];
    char request_body[SIZE];
    char *request_filed[N][M];
    char method[SIZE];
    char target[SIZE]; //SIZE個もいらないwww
    int status;
    unsigned int file_size;
    char *dptr[N][M];
    int send_size;

    request_size = recv(psock, request_message, SIZE, 0);
    if (request_size == -1) {
        printf("recvRequestMessage error\n");
        return 0;
    }else if (request_size == 0) {
        /* 受信サイズが0の場合は相手が接続閉じていると判断 */
        printf("connection ended\n");
        return 0;
    }
    printf("%s\n",request_message);
    //情報を解析する
    if (parseRequestMessage(method,target,request_filed,request_message,request_body) == -1) {
            printf("parseRequestMessage error\n");
            return 0;
    }
/* メソッドがGET以外はステータスコードを404にする */
    if (strcmp(method, "GET") == 0) {
        if (strcmp(target, "/") == 0) {
            /* /が指定された時は/index.htmlに置き換える */
            strcpy(target, "/index.html");
        }
    } else if (strcmp(method, "POST") == 0){
        printf("POST %s HTTP/1.1\n",target);
        parseBody(request_body);
    }else { 
        printf("error");
        return 0;
    }

    file_size = getFileSize(&target[1]);
    status = getProcessing(body, &target[1]); //targetでpathを渡すbodyにはファイルの内容が入っている
    if(status==404)//get,postされたけどpathがなかった場合
        file_size = getFileSize("index404.html");
    /* ヘッダーフィールド作成*/
    sprintf(header_field, "Content-Length: %u\r\n", file_size);

    /* レスポンスメッセージを作成 */
    response_size = createResponseMessage(response_message, status, header_field, body, file_size);
    if (response_size == -1) {
        printf("createResponseMessage error\n");
        return 0;
    }
    /* レスポンスメッセージを送信する */
    send_size = send(psock, response_message, response_size, 0);
    if (send_size == -1) {
        printf("recvRequestMessage error\n");
        return 0;
    }
    close(psock);
}

int main() {
	int sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	int sock;
        pthread_t thread;


        pthread_mutex_init(&mutex,NULL);
        
        //ソケットの作成
	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if (sock0 < 0) {
		perror("socket");
		return 1;
	}

        //情報をaddr構造体に書き込む
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = INADDR_ANY;

	/*setsockopt(sock0,
	SOL_SOCKET, SO_REUSEADDR, (const char *) &yes, sizeof(yes));
        */
        //bindする
	if (bind(sock0, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
		perror("bind");
		return 1;
	}
        
        //listenする
	if (listen(sock0, 5) != 0) {
		perror("listen");
		return 1;
	}
	
	while (1) {
		len = sizeof(client);

                
                
                //acceptする
		sock = accept(sock0, (struct sockaddr *) &client, &len);
		if (sock < 0) {
			perror("accept");
			break;
		}
                
                if(pthread_create(&thread,NULL,(void *)pthread_socket,&sock)!=0){
                    perror("pthread_create()");
                    exit(1);
                }
                pthread_detach(thread);
	}

	close(sock0);

	return 0;
}

