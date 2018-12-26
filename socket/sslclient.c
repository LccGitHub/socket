/**
 * gcc sslclient.c -lssl -lcrypto -o client
 * ./client 127.0.0.1
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>


#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_PORT 1111
#define MAX_BUF_SIZE 1024



#define CERTF       "client_cert.pem"
#define KEYF        "client_key.pem"
#define CACERTF     "rootca_cert.pem"


int main(int argc, char* argv[])
{
	int socketFd = -1;
	int mutualAuth = 0;
    
	if (argc == 2 && strcmp(argv[1], "mutual") == 0) {
		mutualAuth = 1;
		printf("This is one-way authenication mode \n");
		printf("If you want to use two-way authentication mode, please input :\n %s server_ip mutual \n", argv[0]);
	}


	SSL_CTX* ctx = NULL;
	SSL* ssl = NULL;
	const SSL_METHOD *meth;

	SSL_load_error_strings();  // readable error message 
	SSL_library_init();
	//meth = TLS_client_method();
	meth = TLSv1_2_client_method();
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL) {
		printf("SSL_CTX_new failed\n");
		exit(-1);
	}

	if (mutualAuth == 1) {
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

		if (SSL_CTX_load_verify_locations(ctx, CACERTF, 0) != 1) {
			SSL_CTX_free(ctx);
			printf("Failed to load CA file %s \n", CACERTF);
			exit(-1);
		}

		if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
			SSL_CTX_free(ctx);
			printf("Failed to load CA file %s \n", CACERTF);
			exit(-1);
		}

		// loads the first certificate stored in file into ctx
		if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) {
			printf("%s,%d\n", __func__, __LINE__);
			ERR_print_errors_fp(stderr);
			exit(-1);
		}

		// adds the first private key found in file to ctx.
		if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) {
			printf("%s,%d\n", __func__, __LINE__);
			ERR_print_errors_fp(stderr);
			exit(-1);
		}

		// checks the consistency of a private key with the corresponding certificate loaded into ctx
		if (SSL_CTX_check_private_key(ctx) <= 0) {
			printf("Privete key does not match the certificate public key \n");
			exit(-1);
		}

		// sets the list of available cipher
		SSL_CTX_set_cipher_list(ctx, "RC4-MD5");
	}


	SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);





	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd < 0) {
		printf("socket failed, err=%s\n", strerror(errno));
	}
	else {
		struct sockaddr_in addr;
		bzero(&addr, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Server IP */
		addr.sin_port = htons(SERVER_PORT);

		
		if (connect(socketFd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0) {
			printf("connect failed, err = %s\n", strerror(errno));
		}
		else {
			printf("have connect success \r\n");

			/**-----------------have create TCP conneciton, start SSL negotiation--- */

			int err = -1;
			ssl = SSL_new(ctx);
			if (ssl == NULL) {
				printf("SSL_new failed \n");
				exit(-1);
			}
			SSL_set_fd(ssl, socketFd);

			// initiates the TLS/SSL handshake with a server
			err = SSL_connect(ssl);
			if (err == -1) {
				ERR_print_errors_fp(stderr);
				printf("SSL_connect failed \n");
				exit(-1);
			}

			/** -----------Following is optional, only get some server cert info--------------- */
			printf("Client SSL connection using %s\n", SSL_get_cipher(ssl));
			X509* server_cert = SSL_get_peer_certificate(ssl);
			if (server_cert == NULL) {
				printf("SSL_get_peer_certificate failed \n");
			}
			else {
				char *str = X509_NAME_oneline(X509_get_subject_name(server_cert), 0, 0);
				if (str == NULL) {
					printf("X509_NAME_oneline failed \n");
				}
				else {
					printf("\t subject: %s \n", str);
					OPENSSL_free(str);
				}


				str = X509_NAME_oneline(X509_get_issuer_name(server_cert), 0, 0);
				if (str == NULL) {
					printf("X509_NAME_oneline get issuer failed \n");
				}
				else {
					printf("\t issuer: %s \n", str);
					OPENSSL_free(str);
				}
			}

			X509_free(server_cert);

			/** --------------------optional end------------------------- */


			while (1) {
				printf("Please input send msg:\n");
				char buffer[MAX_BUF_SIZE] = { 0 };
				fgets(buffer, MAX_BUF_SIZE, stdin);
				int res = write(socketFd, buffer, strlen(buffer));
				printf("sendto res = %d\n", res);
				bzero(buffer, MAX_BUF_SIZE);
				res = read(socketFd, buffer, MAX_BUF_SIZE);
				printf("client recvfrom res = %d, %s\n", res, buffer);
				bzero(buffer, MAX_BUF_SIZE);
			}
		}

	}
	close(socketFd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);

	return 0;
}

