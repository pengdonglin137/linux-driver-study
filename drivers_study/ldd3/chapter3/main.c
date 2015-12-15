#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define V (2000)
#define H (4000)

char buff[V][H];

int main(int argc, const char *argv[])
{
	FILE *fp;
	int i, j, flag = 0;

	if ((fp = fopen("/dev/scull0", "w+")) == NULL) {
		perror("fail to fopen /dev/scull0\n");
		exit(-1);
	}

	for (i=0; i<V; i++)
		for (j=0; j<H; j++)
			buff[i][j]=0x55;

	fwrite(buff, H, V, fp);
	memset(buff, 0x0, sizeof(buff));
	fclose(fp);

	if ((fp = fopen("/dev/scull0", "r")) == NULL) {
		perror("fail to fopen /dev/scull0\n");
		exit(-1);
	}

	fread(buff, H, V, fp);

	for (i=0; i<V; i++)
		for (j=0; j<H; j++)
		{
#if 0
			if (j%50 == 0) {
				printf("\n");
			}
			printf("0x%x ", buff[i][j]);
#endif
			if (buff[i][j] != 0x55) {
				printf("buff[%d][%d]=%d\n", i, j, buff[i][j]);
				flag = 1;
			}
		}

	fclose(fp);

	if (!flag)
		printf("All equal\n");

	return 0;
}
