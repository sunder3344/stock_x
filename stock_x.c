#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "util.c"
#include <curl/curl.h>
#include "cJSON.h"

char * res;

size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
	//printf("ptr = %s\n", ptr);
	//printf("userdata = %s\n", userdata);
    if (res == NULL) {
	    res = (char *)realloc(res, sizeof(char) * (strlen(ptr)+1));
	    strcpy(res, ptr);
    } else {
        res = (char *)realloc(res, sizeof(char) * (strlen(res) + strlen(ptr) + 1));
        strncat(res, ptr, strlen(ptr));
    }
	//printf("res_callback:=%s\n", res);
	return size * nmemb;
}

int main(int argc, char *argv[]) {
	res = (char *)malloc(sizeof(char) * 1000);
    res = NULL;
	//read stock code from config
	FILE *fp = fopen("config_x.ini", "r");
	if (NULL == fp) {
		perror("Open config file failed!");
	}
	
	//输出标题
	printf("%-15s|%-15s|%-15s|%-15s|%-15s|%-20s|%-20s|%-20s|%-20s|\n", "名称", "当前", "涨幅(%)", "昨收", "今开", "当日最高", "当日最低", "成交数(手)", "成交金额(万元)");

	char line[10];
	char * stockcode;
	stockcode = (char *)malloc(sizeof(char) * 10);
	stockcode = 0;
	char *flag = ",";
	while (!feof(fp)) {
		memset(line, 0, sizeof(line));
		if (fgets(line, sizeof(line), fp) != NULL) {
			int len = strlen(line);
			//printf("line:=%s, stockcode:=%s\n", line, stockcode);
			if (stockcode == 0) {
				stockcode = (char *)realloc(stockcode, sizeof(char) * (len + 1));
				strncpy(stockcode, line, len - 1);
			} else {
				int bigLen = strlen(stockcode);
				stockcode = (char *)realloc(stockcode, sizeof(char) * (bigLen + 1));
				strncat(stockcode, flag, 1);
				bigLen = strlen(stockcode);
				stockcode = (char *)realloc(stockcode, sizeof(char) * (bigLen + len + 1));
				strncat(stockcode, line, len - 1);
			}
		}
	}
	fclose(fp);

	CURL * curl = curl_easy_init();
	CURLcode code;
	char * url = NULL;
	char * sector = "https://stock.xueqiu.com/v5/stock/realtime/quotec.json?symbol=";
	url = (char *)malloc(sizeof(char) * (strlen(sector) + 1));
	strncpy(url, sector, strlen(sector));
	//printf("urlurlurl:=%s, len=%d\n\n", url, strlen(sector));
	int urllen = strlen(url);
	url = (char *)realloc(url, sizeof(char) * (urllen + strlen(stockcode) + 1));
	strncat(url, stockcode, strlen(stockcode));
	//printf("url:=%s\n", url);

	struct curl_slist * head = NULL;
	curl_slist_append(head, "authority: stock.xueqiu.com");
	curl_slist_append(head, "method: GET");
	curl_slist_append(head, "scheme: https");
	curl_slist_append(head, "accept-encoding: gzip, deflate, br");
	curl_slist_append(head, "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36");
	

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	code = curl_easy_perform(curl);
	curl_slist_free_all(head);
	curl_easy_cleanup(curl);
	//cjson
    //printf("res:=%s\n", res);
    cJSON *parse = cJSON_Parse(res);
    const cJSON *data = NULL;
    const cJSON *item = NULL;
    const cJSON *codes = NULL;
    const cJSON *desc = NULL;

    codes = cJSON_GetObjectItemCaseSensitive(parse, "error_code");
    if (cJSON_IsNumber(codes) && codes->valuedouble == 0) {
        //printf("code:=%d\n", codes->valueint);
        data = cJSON_GetObjectItemCaseSensitive(parse, "data");
        cJSON_ArrayForEach(item, data) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "symbol");
            cJSON *current = cJSON_GetObjectItemCaseSensitive(item, "current");
            cJSON *percent = cJSON_GetObjectItemCaseSensitive(item, "percent");
            cJSON *last_close = cJSON_GetObjectItemCaseSensitive(item, "last_close");
            cJSON *open = cJSON_GetObjectItemCaseSensitive(item, "open");
            cJSON *high = cJSON_GetObjectItemCaseSensitive(item, "high");
            cJSON *low = cJSON_GetObjectItemCaseSensitive(item, "low");
            cJSON *volume = cJSON_GetObjectItemCaseSensitive(item, "volume");
            cJSON *amount = cJSON_GetObjectItemCaseSensitive(item, "amount");

            printf("%-13s|%-13.2f|%-13.2f|%-13.2f|%-13.2f|%-16.2f|%-16.2f|%-16d|%-14d|\n", name->valuestring, current->valuedouble, percent->valuedouble, last_close->valuedouble, open->valuedouble, high->valuedouble, low->valuedouble, volume->valueint/100, amount->valueint/10000);
        }
    }

    cJSON_Delete(parse);
    free(stockcode);
	free(res);
	free(url);
	return 0;
}
