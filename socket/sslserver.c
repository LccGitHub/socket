/**
 * gcc sslserver.c -lssl -lcrypto -lpthread -o server
 * ./server
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
#include <pthread.h>


#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>




#define CERTF       "server_cert.pem"
#define KEYF        "server_key.pem"
#define CACERTF     "rootca_cert.pem"


#define SERVER_PORT 1111
#define MAX_BUF_SIZE 1024

void* clientThread(void* arg)
{
	int newFd = *((int*)arg);
	printf("clientThread start, newFd = %d \n", newFd);
	while (1) {
		char buffer[MAX_BUF_SIZE] = { 0 };
		int addrlen = sizeof(struct sockaddr);
		int res = read(newFd, buffer, MAX_BUF_SIZE);
		printf("server recv res = %d, msg[%s]\n", res, buffer);
		bzero(buffer, MAX_BUF_SIZE);
		res = write(newFd, "server received", 20);
		printf("server send res = %d\n", res);
		bzero(buffer, MAX_BUF_SIZE);
	}
	return NULL;
}


int main(int argc, char **argv)
{
	int socketFd = -1;
	int mutualAuth = 0;

	if (argc == 2 && strcmp(argv[1], "mutual") == 0) {
		printf("Enable mutual authentication\n");
		mutualAuth = 1;
	}
	else {
		printf("This is one-way authentication mode\n");
		printf("If you want to use two-way authentication mode, please input:\n ./server mutual \n");
	}

	SSL_CTX* ctx = NULL;
	SSL* ssl = NULL;
	const SSL_METHOD *meth;

	SSL_load_error_strings();  // readable error message 
	SSLeay_add_ssl_algorithms();
	// OpenSSL_add_ssl_algorithms() and SSLeay_add_ssl_algorithms() are synonyms for SSL_library_init().
	//SSL_library_init(); // need call befor any action
	//meth = TLS_serverserver_method();
	meth = TLSv1_2_server_method();
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL) {
		printf("SSL_CTX_new failed\n");
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	if (mutualAuth == 1) {
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);

		if (SSL_CTX_load_verify_locations(ctx, CACERTF, 0) != 1) {
			SSL_CTX_free(ctx);
			printf("Failed to load CA file %s \n", CACERTF);
			exit(-1);
		}

		if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
			SSL_CTX_free(ctx);
			printf("Call to SSL_CTX_set_default_verify_paths failed \n");
			exit(-1);
		}

	}


	// loads the first certificate stored in file into ctx
	if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	// adds the first private key found in file to ctx.
	if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) {
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
	SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);



	socketFd = socket(AF_INET, SOCK_STREAM, 0); // AF_NET:ipv4, SOCK_STREAM:tcp
	if (socketFd < 0) {
		printf("socket failed, err=%s\n", strerror(errno));
	}
	else {

		int flag = 1;
		setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));


		struct sockaddr_in addr;
		bzero(&addr, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		//addr.sin_addr.s_addr = htonl(INADDR_ANY);
		// addr.sin_addr.s_addr = inet_addr("192.168.0.1"); 
		addr.sin_port = htons(SERVER_PORT);


		if (bind(socketFd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
			printf("bind failed, err = %s\n", strerror(errno));
		}
		else if (listen(socketFd, 5) < 0) {
			printf("listen err = %s\n", strerror(errno));
		}
		else {
			while(1) {
				struct sockaddr_in clientAddr;
				int sinLen = sizeof(struct sockaddr_in);
				int newFd = accept(socketFd, (struct sockaddr*)&clientAddr, &sinLen);

				printf ("Connection from fd=%d, port %x\n", newFd, clientAddr.sin_port);
				/**-----------------have create TCP conneciton, start SSL negotiation--- */

				int err = -1;
				ssl = SSL_new(ctx);
				if (ssl == NULL) {
					printf("SSL_new failed \n");
					exit(-1);
				}
				SSL_set_fd(ssl, newFd);

				// initiates the TLS/SSL handshake with a server
				err = SSL_accept(ssl);
				if (err == -1) {
					ERR_print_errors_fp(stderr);
					printf("SSL_accept failed , err = %d\n", err);
					exit(-1);
				}

				/** -----------Following is optional, only get some server cert info--------------- */
				printf("Server SSL connection using %s\n", SSL_get_cipher(ssl));
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



				pthread_t cThread;
				pthread_create(&cThread, NULL, &clientThread, &newFd);
			}
		}

	}
	close(socketFd);

}

