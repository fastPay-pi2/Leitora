/* file:          reader_api.c
 * 
 * description:   Gen2 RFID reader host software API.
 *                This API uses the BSD sockets.
 * 
 * date:          12/2013
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 * 
 * date: 		  05/2019
 * author:		  Eduardo Gonçalves Sousa <eng.eduardo.gs@gmail.com>
 * 
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "reader_api.h"

#define h_addr h_addr_list[0] /* for backward compatibility */

/*
Gen2 RFID reader C language API
*/
int32_t Gen2ReaderConfig(uint8_t *addr, readerconfig_t *new_config){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	int32_t sockfd, numbytes, addr_len;
	struct sockaddr_in their_addr;
	struct hostent *he;
	struct timeval tv;
	uint32_t trid;

	tv.tv_sec = UDP_COMM_TIMEOUT;
	tv.tv_usec = 1000 * UDP_COMM_TIMEOUT_MS;

	if ((he=gethostbyname(addr)) == NULL){
		return ERR_SOCKET_ERROR;
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		return ERR_SOCKET_ERROR;
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(UDP_PORT_COMMAND);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
		return ERR_SOCKET_ERROR;
	}

	srand((uint32_t)time(NULL));
	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=config");
	strcat(buf, "?ip_addr="); strcat(buf, new_config->ip_addr);
	strcat(buf, "?mask_addr="); strcat(buf, new_config->mask_addr);
	strcat(buf, "?gate_addr="); strcat(buf, new_config->gate_addr);
	strcat(buf, "?tx_power="); sprintf(buf2, "%d", new_config->tx_power); strcat(buf, buf2);
	strcat(buf, "?blf="); sprintf(buf2, "%d", new_config->blf); strcat(buf, buf2);
	strcat(buf, "?encoding="); sprintf(buf2, "%d", new_config->encoding); strcat(buf, buf2);
	strcat(buf, "?divr="); sprintf(buf2,  "%d",new_config->dr); strcat(buf, buf2);
	strcat(buf, "?anti_collision_mode="); sprintf(buf2, "%d", new_config->anti_collision_q[0]); strcat(buf, buf2);
	strcat(buf, "?q_start="); sprintf(buf2, "%d", new_config->anti_collision_q[1]); strcat(buf, buf2);
	strcat(buf, "?q_min="); sprintf(buf2, "%d", new_config->anti_collision_q[3]); strcat(buf, buf2);
	strcat(buf, "?q_max="); sprintf(buf2, "%d", new_config->anti_collision_q[2]); strcat(buf, buf2);
	strcat(buf, "?");

	if ((numbytes = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}
	
	sleep(6);

	addr_len = sizeof(struct sockaddr);
	if ((numbytes = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&their_addr, (socklen_t *)&addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	close(sockfd);

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t Gen2ReaderCreate(uint8_t *addr, reader_t *session){
	session->tv.tv_sec = UDP_COMM_TIMEOUT;
	session->tv.tv_usec = 1000 * UDP_COMM_TIMEOUT_MS;

	if ((session->he=gethostbyname(addr)) == NULL){
		return ERR_SOCKET_ERROR;
	}

	if ((session->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		return ERR_SOCKET_ERROR;
	}

	session->their_addr.sin_family = AF_INET;
	session->their_addr.sin_port = htons(UDP_PORT_COMMAND);
	session->their_addr.sin_addr = *((struct in_addr *)session->he->h_addr);
	memset(&(session->their_addr.sin_zero), '\0', 8);

	if (setsockopt(session->sockfd, SOL_SOCKET, SO_RCVTIMEO, &session->tv, sizeof(session->tv)) < 0){
		return ERR_SOCKET_ERROR;
	}

	srand((uint32_t)time(NULL));

	return ERR_OK;
}

int32_t Gen2ReaderDestroy(reader_t *session){
	close(session->sockfd);
	return ERR_OK;
}

int32_t Gen2ReaderGPIODirection(reader_t *session, uint8_t port, uint8_t pins){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=gpio_dir?");
	strcat(buf, "?port="); sprintf(buf2, "%d", port); strcat(buf, buf2);
	strcat(buf, "?value="); sprintf(buf2, "%d", pins); strcat(buf, buf2);
	strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session-> numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	if (((buf[GEN2_RESP_OFS] << 8) | buf[GEN2_RESP_OFS + 1]) > 0)
		return ERR_SOCKET_WRONGDATA;

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t Gen2ReaderGPIOWrite(reader_t *session, uint8_t port, uint8_t pins){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=gpio_set?");
	strcat(buf, "?port="); sprintf(buf2, "%d", port); strcat(buf, buf2);
	strcat(buf, "?value="); sprintf(buf2, "%d", pins); strcat(buf, buf2);
	strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session-> numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	if (((buf[GEN2_RESP_OFS] << 8) | buf[GEN2_RESP_OFS + 1]) > 0)
		return ERR_SOCKET_WRONGDATA;

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t Gen2ReaderGPIORead(reader_t *session, uint8_t port, uint8_t *pins){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=gpio_get?");
	strcat(buf, "?port="); sprintf(buf2, "%d", port); strcat(buf, buf2);
	strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session-> numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	if (((buf[GEN2_RESP_OFS] << 8) | buf[GEN2_RESP_OFS + 1]) != 1)
		return ERR_SOCKET_WRONGDATA;

	*pins = buf[HEADER_SIZE];

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t Gen2ReaderNetworkStats(reader_t *session, ip_stack_t *stats){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t i, trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=stats?network?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session-> numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	if (((buf[GEN2_RESP_OFS] << 8) | buf[GEN2_RESP_OFS + 1]) != sizeof(ip_stack_data))
		return ERR_SOCKET_WRONGDATA;

	for (i = 0; i < sizeof(ip_stack_data); i++)
		stats->raw_data[i] = buf[HEADER_SIZE + i];

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t Gen2ReaderGen2Stats(reader_t *session, gen2_t *stats){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t i, trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=stats?gen2?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session-> numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	if (((buf[GEN2_RESP_OFS] << 8) | buf[GEN2_RESP_OFS + 1]) != sizeof(gen2_data))
		return ERR_SOCKET_WRONGDATA;

	for (i = 0; i < sizeof(gen2_data); i++)
		stats->raw_data[i] = buf[HEADER_SIZE + i];

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t Gen2ReaderReset(reader_t *session){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=reset");
	strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session-> numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	if (((buf[GEN2_RESP_OFS] << 8) | buf[GEN2_RESP_OFS + 1]) > 0)
		return ERR_SOCKET_WRONGDATA;

	return (int16_t)((buf[CMD_RESP_OFS] << 8) | buf[CMD_RESP_OFS + 1]);
}

int32_t SetAntenna(reader_t *session, uint8_t arg){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t i, trid;
	
	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);

	strcat(buf, "?reader?command=set_antenna");

	strcat(buf, "?antenna=");
	sprintf(buf2, "%d", arg); strcat(buf, buf2);
	strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}
	
	session->addr_len = sizeof(struct sockaddr);
	if ((session->numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	return ERR_OK;
}

int32_t GetVersion(reader_t *session, uint8_t *version_rf, uint8_t *version_digital){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t i, trid;
	uint8_t size_version_rf, size_version_digital;
	
	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);

	strcat(buf, "?reader?command=get_version");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}
	
	session->addr_len = sizeof(struct sockaddr);
	if ((session->numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	size_version_rf = buf[BIT_LENGTH_VERSION_RF];
	size_version_digital = buf[BIT_LENGTH_VERSION_DIGITAL];

	memcpy(&version_rf[0], &buf[BIT_START_RESPONSE], size_version_rf);
	memcpy(&version_digital[0], &buf[BIT_START_RESPONSE+size_version_rf], size_version_digital);

	return ERR_OK;
}

int32_t handle_rfid_module(reader_t *session, _rcp *msg, _rcp *response, tag1_t *tag_scan1, tag2_t *tag_scan2, tag3_t *tag_scan3, tag4_t *tag_scan4, tag5_t *tag_scan5, tag6_t *tag_scan6){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t i, trid;

	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);
	strcat(buf, "?reader?command=gen2?code="); sprintf(buf2, "%02x", msg->code); strcat(buf, buf2);
	strcat(buf, "?pl_len="); sprintf(buf2, "%02x", msg->payload_length[1]); strcat(buf, buf2);
	if(msg->payload_length[1] > 0)
	{
		strcat(buf, "?pl=");
		for(i = 0; i < msg->payload_length[1]; i++)
		{
			sprintf(buf2, "%02x", msg->payload[i]); 
			strcat(buf, buf2);
		}
	}
	strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}

	session->addr_len = sizeof(struct sockaddr);
	if ((session->numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	
	// Receive response
	response->payload_length[0] = 0;
	response->payload_length[1] = buf[BIT_RESPONSE_LENGTH];
	response->code = buf[BIT_RESPONSE_CODE];
	
	memcpy(&response->payload[0], &buf[BIT_START_RESPONSE], response->payload_length[1]);
	
	if(response->msg_type == MSG_TYPE_ERROR)
	{
		return ERR_SOCKET_WRONGDATA;
	}
	else
	{
		//Receive Notification
		if(msg->code == READ_TYPE_C_UII_TID || msg->code == READ_TYPE_C_TAG_LONG_DATA || msg->code == START_AUTO_READ2  || msg->code == START_AUTO_READ_ARTEFATO || msg->code == START_AUTO_READ_RSSI || msg->code == START_AUTO_READ_RSSI_ARTEFATO)
		{
			switch(msg->code){
				case START_AUTO_READ2:
					num_tags = buf[BIT_NUM_TAGS];
					for (i = 0; (i < (((buf[BIT_ALL_TAGS_LENGTH_MSB] << 8) | buf[BIT_ALL_TAGS_LENGTH_LSB]))) && (i < sizeof(tag1_t)); i++)
						tag_scan1->raw_data[i] = buf[BIT_START_TAGS + i];
					return num_tags;
				break;

				case START_AUTO_READ_ARTEFATO:
					num_tags = buf[BIT_NUM_TAGS];
					for (i = 0; (i < (((buf[BIT_ALL_TAGS_LENGTH_MSB] << 8) | buf[BIT_ALL_TAGS_LENGTH_LSB]))) && (i < sizeof(tag5_t)); i++)
						tag_scan5->raw_data[i] = buf[BIT_START_TAGS + i];
					return num_tags;
				break;
				
				case START_AUTO_READ_RSSI:
					num_tags = buf[BIT_NUM_TAGS];
					for (i = 0; (i < (((buf[BIT_ALL_TAGS_LENGTH_MSB] << 8) | buf[BIT_ALL_TAGS_LENGTH_LSB]))) && (i < sizeof(tag2_t)); i++)
						tag_scan2->raw_data[i] = buf[BIT_START_TAGS + i];
					return num_tags;
				break;

				case START_AUTO_READ_RSSI_ARTEFATO:
					num_tags = buf[BIT_NUM_TAGS];
					for (i = 0; (i < (((buf[BIT_ALL_TAGS_LENGTH_MSB] << 8) | buf[BIT_ALL_TAGS_LENGTH_LSB]))) && (i < sizeof(tag5_t)); i++)
						tag_scan6->raw_data[i] = buf[BIT_START_TAGS + i];
					return num_tags;
				break;
				
				case READ_TYPE_C_UII_TID:
					num_tags = buf[BIT_NUM_TAGS];
					for (i = 0; (i < (((buf[BIT_ALL_TAGS_LENGTH_MSB] << 8) | buf[BIT_ALL_TAGS_LENGTH_LSB]))) && (i < sizeof(tag3_t)); i++)
						tag_scan3->raw_data[i] = buf[BIT_START_TAGS + i];
					return num_tags;
				break;

				case READ_TYPE_C_TAG_LONG_DATA:
					//TO DO
				break;		
			}
		}
	}	
	//return  (int16_t)((buf[BIT_ALL_TAGS_LENGTH_MSB] << 8) | buf[BIT_ALL_TAGS_LENGTH_LSB]) / sizeof(_tag1);	
	return ERR_OK;
}

int32_t continuousReading(reader_t *session, uint8_t arg){
	uint8_t buf[HOSTMSG_SIZE], buf2[20];
	uint32_t i, trid;
	
	trid = random();

	strcpy(buf, "?trid="); sprintf(buf2, "%08x", trid); strcat(buf, buf2);

	if(arg == 0)
		strcat(buf, "?reader?command=continuous_reading_off");
	if(arg == 1)
		strcat(buf, "?reader?command=continuous_reading_on");
	

	// strcat(buf, "?antenna=");
	// sprintf(buf2, "%d", arg); strcat(buf, buf2);
	// strcat(buf, "?");

	if ((session->numbytes = sendto(session->sockfd, buf, strlen(buf), 0, (struct sockaddr *)&session->their_addr, sizeof(struct sockaddr))) == -1){
		return ERR_SOCKET_SEND;
	}
	
	session->addr_len = sizeof(struct sockaddr);
	if ((session->numbytes = recvfrom(session->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&session->their_addr, (socklen_t *)&session->addr_len)) == -1){
		return ERR_SOCKET_NORESP;
	}

	if (~((buf[TRID_OFS] << 24) | (buf[TRID_OFS+1] << 16) | (buf[TRID_OFS+2] << 8) | (buf[TRID_OFS+3])) != trid)
		return ERR_SOCKET_TRID;

	return ERR_OK;
}