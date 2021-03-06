/* ********** ********** ********** ********** ********** ********** ********** ********** ********** **********
shbaek: Include File
********** ********** ********** ********** ********** ********** ********** ********** ********** ********** */
#include "grib_http.h"

/* ********** ********** ********** ********** ********** ********** ********** ********** ********** **********
shbaek: Global Variable
********** ********** ********** ********** ********** ********** ********** ********** ********** ********** */
int  gHttpDebug = FALSE;
int  gHttpTombStone = FALSE;

//2 shbaek: Return Recv Byte
char gLineBuff[HTTP_MAX_SIZE];
/* ********** ********** ********** ********** ********** ********** ********** ********** ********** ********** */
#define __HTTP_FUNC__
/* ********** ********** ********** ********** ********** ********** ********** ********** ********** ********** */
void Grib_HttpSetDebug(int iDebug, int iTombStone)
{
	if(iDebug == TRUE)
	{
		gHttpDebug = TRUE;
		GRIB_LOGD("# HTTP DEBUG: ON\n");
	}

	if(iTombStone == TRUE)
	{
		gHttpTombStone = TRUE;
		GRIB_LOGD("# HTTP TOMBSTONE: ON\n");
	}

	return;
}

void Grib_HttpTombStone(Grib_HttpLogInfo* pLogInfo)
{
	time_t sysTimer;
	struct tm *sysTime;

	sysTimer = time(NULL);
	sysTime  = localtime(&sysTimer);

	int   iFD = 0;
    char  pLogFilePath[SIZE_1K] = {'\0', };
	char* pMsg = NULL;

	if(gHttpTombStone != TRUE)
	{//shbaek: Use Not TombStone
		return;
	}

    SNPRINTF(pLogFilePath, sizeof(pLogFilePath), "%s/HTTP_%04d%02d%02d_%02d%02d%02d_%s.log", GRIB_FILE_PATH_LOG_ROOT, 
												 sysTime->tm_year+1900, sysTime->tm_mon+1, sysTime->tm_mday,
												 sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec, pLogInfo->httpSender);

	GRIB_LOGD("# %s-HTTP: # ##### ##### ##### ##### ##### ##### #####\n", pLogInfo->httpSender);
	GRIB_LOGD("# %s-HTTP: # LOG NAME: %c[1;31m%s%c[0m\n", pLogInfo->httpSender, 27, pLogFilePath, 27);
	GRIB_LOGD("# %s-HTTP: # LOG MSG : %c[1;31m%s%c[0m\n", pLogInfo->httpSender, 27, pLogInfo->httpErrMsg, 27);
	GRIB_LOGD("# %s-HTTP: # ##### ##### ##### ##### ##### ##### #####\n", pLogInfo->httpSender);

	iFD = open(pLogFilePath, O_WRONLY|O_CREAT, 0666);

	pMsg = "# SENDER NAME: ";
	write(iFD, pMsg, STRLEN(pMsg));
	write(iFD, pLogInfo->httpSender, STRLEN(pLogInfo->httpSender));
	write(iFD, GRIB_CRLN, STRLEN(GRIB_CRLN));

	pMsg = "# SEND MSG   : ";
	write(iFD, pMsg, STRLEN(pMsg));
	write(iFD, GRIB_CRLN, STRLEN(GRIB_CRLN));
	write(iFD, pLogInfo->httpSendMsg, STRLEN(pLogInfo->httpSendMsg));
	write(iFD, GRIB_CRLN, STRLEN(GRIB_CRLN));

	pMsg = "# ERROR MSG  : ";
	write(iFD, pMsg, STRLEN(pMsg));
	write(iFD, pLogInfo->httpErrMsg, STRLEN(pLogInfo->httpErrMsg));
	write(iFD, GRIB_CRLN, STRLEN(GRIB_CRLN));

	close(iFD);
}

int Grib_HttpConnect(char* serverIP, int serverPort)
{
	int iFD		= GRIB_ERROR;
	int iRes	= GRIB_ERROR;

	timeval timeOut;
	struct sockaddr_in serverAddr;

	iFD = socket(AF_INET, SOCK_STREAM, 0);
	if(iFD < 0)
	{
		GRIB_LOGD("SOCKET OPEN FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		return GRIB_ERROR;
	}

	timeOut.tv_sec  = HTTP_TIME_OUT_SEC_RECEIVE;
	timeOut.tv_usec = GRIB_NOT_USED;
	setsockopt(iFD, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeOut, sizeof(timeval));

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);
	serverAddr.sin_port = htons(serverPort);

	iRes = connect(iFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(iRes != GRIB_SUCCESS)
	{
		GRIB_LOGD("HTTP CONNECT FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		return GRIB_ERROR;
	}

	return iFD;
}

int Grib_HttpConnectTimeOut(char* serverIP, int serverPort)
{
	int iFD		= GRIB_ERROR;
	int iRes	= GRIB_ERROR;

	int iSize	= 0;
	int iCount	= 0;
	int iError	= 0;

	int nonSocketStat = 0;
	int orgSocketStat = 0;

	fd_set  readSet;
	fd_set  writeSet;

	timeval recvTimeOut;
	timeval connTimeOut;
	struct sockaddr_in serverAddr;

	iFD = socket(AF_INET, SOCK_STREAM, GRIB_NOT_USED);
	if(iFD < 0)
	{
		GRIB_LOGD("# SOCKET OPEN FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		return GRIB_ERROR;
	}

	//shbaek: Get Socket Stat
	orgSocketStat = fcntl(iFD, F_GETFL, NULL);
	if(orgSocketStat < 0)
	{
		GRIB_LOGD("# GET FILE STAT FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		return GRIB_ERROR;
	}

	//shbaek: Set Non-Block Stat
	nonSocketStat = orgSocketStat | O_NONBLOCK;
	iRes = fcntl(iFD, F_SETFL, nonSocketStat);
	if(iRes < 0)
	{
		GRIB_LOGD("# SET NONBLOCK STAT FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		return GRIB_ERROR;
	}

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);
	serverAddr.sin_port = htons(serverPort);

	//shbaek: Connect
	iRes = connect(iFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(iRes < 0)
	{
		if(LINUX_ERROR_NUM != EINPROGRESS)
		{//shbaek: Connect Error
			GRIB_LOGD("# HTTP CONNECT FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
			return GRIB_ERROR;
		}
	}
	else
	{//shbaek: One Shot Connected -> Restore Socket Stat
		fcntl(iFD, F_SETFL, orgSocketStat);
		return iFD;
	}

    FD_ZERO(&readSet);
    FD_SET(iFD, &readSet);
    writeSet = readSet;
	connTimeOut.tv_sec  = HTTP_TIME_OUT_SEC_CONNECT;
	connTimeOut.tv_usec = GRIB_NOT_USED;

	//shbaek: Wait Read & Write Set
	iCount = select(iFD+1, &readSet, &writeSet, GRIB_NOT_USED, &connTimeOut);
	if(iCount == 0)
	{
		//shbaek: Time Out
		LINUX_ERROR_NUM = ETIMEDOUT;
		GRIB_LOGD("# SELECT TIME OUT: %d SEC\n", HTTP_TIME_OUT_SEC_CONNECT);
		return GRIB_ERROR;
	}

	//shbaek: Check Read & Write Set
    if( FD_ISSET(iFD, &readSet) || FD_ISSET(iFD, &writeSet) ) 
    {
    	iSize = sizeof(int);
		iCount = getsockopt(iFD, SOL_SOCKET, SO_ERROR, &iError, (socklen_t *)&iSize);
		if(iCount < 0)
		{
			GRIB_LOGD("# GET SOCKET OPTION FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
			return GRIB_ERROR;
		}
    } 
    else 
    {
		//shbaek: Nobody Has Touched
		GRIB_LOGD("# NOBODY HAS TOCHED SOCKET\n");
        return GRIB_ERROR;
    }

	//shbaek: Restore Socket Stat
    fcntl(iFD, F_SETFL, orgSocketStat);
    if(iError)
    {
        LINUX_ERROR_NUM = iError;
		GRIB_LOGD("# SET FILE STAT FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
        return GRIB_ERROR;
    }

	//shbaek: Set Receive Time Out
	recvTimeOut.tv_sec  = HTTP_TIME_OUT_SEC_RECEIVE;
	recvTimeOut.tv_usec = GRIB_NOT_USED;
	setsockopt(iFD, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeOut, sizeof(timeval));

	return iFD;
}

int Grib_HttpResParser(Grib_HttpMsgInfo* pMsg)
{
	int i = 0;
	int iRes = GRIB_ERROR;
	int iLoopMax = 128;
	int iDBG = gHttpDebug;

	char* strToken		= NULL;
	char* str1Line		= NULL;
	char* strResponse	= NULL;

	char* strHttpVer	= NULL;
	char* strCodeNum	= NULL;
	char* strCodeMsg	= NULL;

	if( (pMsg==NULL) || (pMsg->recvBuff==NULL) )
	{
		GRIB_LOGD("# PARAM IS NULL\n");
		return GRIB_ERROR;
	}

	//shbaek: Set Init.
	pMsg->statusCode = HTTP_STATUS_CODE_UNKNOWN;
	STRINIT(pMsg->statusMsg, sizeof(pMsg->statusMsg));

	strToken = GRIB_CRLN;
	strResponse = STRDUP(pMsg->recvBuff);
	if(strResponse == NULL)
	{
		GRIB_LOGD("# RESPONSE COPY ERROR\n");
		goto FINAL;
	}

	str1Line = STRTOK(strResponse, strToken);
	if(str1Line == NULL)
	{
		GRIB_LOGD("# 1ST LINE IS NULL !!!\n");
		goto FINAL;
	}

	iRes = STRNCMP(str1Line, HTTP_VERSION_1P1, STRLEN(HTTP_VERSION_1P1));
	if(iRes != 0)
	{//shbaek: is not Same? Something Wrong
		GRIB_LOGD("# 1ST LINE IS WRONG: %s\n", str1Line);
		goto FINAL;
	}

/**
 * shbaek: Ex) Response Status Code
 *			HTTP/1.1 200 OK
 *			HTTP/1.1 404 Not Found
 *			[1:Space] [3:Number Count]
 */
	
	strHttpVer = str1Line;
	strCodeNum = strHttpVer+STRLEN(HTTP_VERSION_1P1)+1;
	strCodeMsg = strCodeNum+3+1;

	*(strCodeNum-1) = NULL;
	*(strCodeMsg-1) = NULL;

	pMsg->statusCode= ATOI(strCodeNum);

	STRINIT(pMsg->statusMsg, sizeof(pMsg->statusMsg));
	STRNCPY(pMsg->statusMsg, strCodeMsg, STRLEN(strCodeMsg));

FINAL:
	if(strResponse!=NULL)FREE(strResponse);

	return GRIB_DONE;
}

int Grib_Recv1Line(int iFD, char* lineBuff, int buffSize, int opt)
{
	int iCount = 0;
	int iDBG = FALSE;

	char prevChar = '\0';
	char readChar = '\0';

	while(recv(iFD, &readChar, sizeof(readChar), GRIB_NOT_USED) == sizeof(readChar))
	{
		lineBuff[iCount] = readChar;
		iCount++;

		if( (prevChar==GRIB_CR) && (readChar==GRIB_LN) )
		{//shbaek: Cut Line
			break;
		}

		prevChar = readChar;

		if(buffSize <= iCount)
		{//shbaek: Limit Buff Size
			break;
		}
	}

	if(iDBG)GRIB_LOGD("# RECV 1LINE: COUNT: %d\n", iCount);

	return iCount;
}

int Grib_RecvChunked(int iFD, char* recvBuff, int buffSize, int opt)
{
	int iDBG = FALSE;
	int iRes = GRIB_ERROR;
	int iCount = 0;
	int iTotal = 0;
	int recvSize = 0;

	char* pEndPoint = "\r\n";

	do{
		
		STRINIT(gLineBuff, sizeof(gLineBuff));
		if(recvSize == 0)recvSize = buffSize;
		else recvSize = recvSize+STRLEN(GRIB_CRLN);

		iCount = Grib_Recv1Line(iFD, gLineBuff, recvSize, GRIB_NOT_USED);
		if(iCount < 0)
		{//shbaek: Recv Error
			GRIB_LOGD("# RECV CHUNKED FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
			return GRIB_FAIL;
		}
		else if( (iCount==3) && (gLineBuff[0]=='0') )
		{//shbaek: Recv End
			if(iDBG)GRIB_LOGD("# NO MORE RECV DATA\n");

			recvBuff[iTotal++]= GRIB_CR;
			recvBuff[iTotal++]= GRIB_LN;

			break;
		}
		else if( (iCount <= HTTP_MAX_SIZE_CHUNKED_HEX_STR) && Grib_isHexString(gLineBuff, iCount-STRLEN(GRIB_CRLN)))
		{//shbaek: Size Data
			recvSize = strtol(gLineBuff, &pEndPoint, 16);
			if(iDBG)GRIB_LOGD("# NEXT RECV SIZE[%d]: %s\n", recvSize, gLineBuff);

		}
		else
		{//shbaek: Real Data
			int iCopySize = iCount;

			if( recvSize != buffSize)
			{
				if( (gLineBuff[iCount-2]=='\r') && (gLineBuff[iCount-1]=='\n') )
				{
					iCopySize -= STRLEN(GRIB_CRLN);
					if(iDBG)GRIB_LOGD("# END IS CR LF\n");
				}				
			}
			MEMCPY(recvBuff+iTotal, gLineBuff, iCopySize);

			iTotal += iCopySize;
			recvSize = 0; //shbaek: for Chunked Data Size
			if(iDBG)
			{
				GRIB_LOGD("# RECV:%d COPY:%d TOTAL:%d\n", iCount, iCopySize, iTotal);
				GRIB_LOGD("# ONLY DATA[%d]: %s\n", iCount, gLineBuff);
			}
		}
	}while(iTotal < buffSize);
	

	return iTotal;
}

int Grib_HttpSendMsg(Grib_HttpMsgInfo* pMsg)
{
	int iCount	= GRIB_ERROR;
	int iTotal	= GRIB_ERROR;
	int iDBG 	= gHttpDebug;

	int iFD 	= 0;
	int iError	= FALSE;

	int isChunk = FALSE;

	int iTimeCheck = gHttpDebug;
	time_t sysTimer;
	struct tm *sysTime;
	Grib_HttpLogInfo httpLogInfo;

	if( pMsg == NULL)
	{
		GRIB_LOGD("# HTTP SEND MSG PARAM NULL ERROR !!!\n");
		return GRIB_ERROR;
	}

	if( (pMsg->serverIP==NULL) || (pMsg->serverPort==0) )
	{
		GRIB_LOGD("# HTTP SERVER INFO NULL ERROR !!!\n");
		return GRIB_ERROR;
	}

	if( STRLEN(pMsg->sendBuff) <= 0)
	{
		GRIB_LOGD("# HTTP SEND BUFFER EMPTY !!!\n");
		return GRIB_ERROR;
	}

	iCount = -1;
	iTotal = 0;
	MEMSET(pMsg->recvBuff, GRIB_INIT, HTTP_MAX_SIZE_RECV_MSG);
	MEMSET(&httpLogInfo, 0x00, sizeof(httpLogInfo));
	if( STRLEN(pMsg->LABEL) <= 0)
	{
		httpLogInfo.httpSender = HTTP_DEFAULT_SENDER;
	}
	else
	{
		httpLogInfo.httpSender = pMsg->LABEL;
	}
	httpLogInfo.httpSendMsg = pMsg->sendBuff;

	//shbaek: Server Connect.
	iFD = Grib_HttpConnectTimeOut(pMsg->serverIP, pMsg->serverPort);
	if(iFD < 0)
	{
		GRIB_LOGD("# HTTP CONNECT ERROR: %s:%d\n", pMsg->serverIP, pMsg->serverPort);
		STRINIT(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg));
		SNPRINTF(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg), "%s [%s (%d)]", "CONNECT ERROR", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		iError = TRUE;
		goto FINAL;
	}

	if(iTimeCheck)
	{//2 shbaek: TIME CHECK
		sysTimer = time(NULL);
		sysTime  = localtime(&sysTimer);
		GRIB_LOGD("# SEND BEGIN TIME: %02d:%02d:%02d\n", sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec);
	}

	iCount = send(iFD, pMsg->sendBuff, STRLEN(pMsg->sendBuff), GRIB_NOT_USED);
	if(iCount <= 0)
	{
		GRIB_LOGD("# SEND FAIL: %s[%d]\n", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		STRINIT(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg));
		SNPRINTF(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg), "%s [%s (%d)]", "SEND ERROR", LINUX_ERROR_STR, LINUX_ERROR_NUM);
		iError = TRUE;
		goto FINAL;
	}
	if(iDBG)GRIB_LOGD("# %s-HTTP-MSG: SEND DONE [TOTAL: %d]\n", httpLogInfo.httpSender, iCount);

	do{
		MEMSET(gLineBuff, 0x00, sizeof(gLineBuff));
//		iCount = recv(iFD, (pMsg->recvBuff+iTotal), (HTTP_MAX_SIZE_RECV_MSG-iTotal), GRIB_NOT_USED);
		iCount = Grib_Recv1Line(iFD, gLineBuff, sizeof(gLineBuff), GRIB_NOT_USED);
		if(iCount < 0)
		{
			GRIB_LOGD("# %s-HTTP-MSG: RECV FAIL: %s[%d]\n", httpLogInfo.httpSender, LINUX_ERROR_STR, LINUX_ERROR_NUM);
			STRINIT(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg));
			SNPRINTF(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg), "%s [%s (%d)]", "RECV ERROR", LINUX_ERROR_STR, LINUX_ERROR_NUM);
			iError = TRUE;
			break;
		}
		else if(iCount == 0)
		{
			if(iTotal == 0)
			{//shbaek: Something Wrong !!!
				STRINIT(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg));
				SNPRINTF(httpLogInfo.httpErrMsg, sizeof(httpLogInfo.httpErrMsg), "%s [%s (%d)]", "RECV NULL", LINUX_ERROR_STR, LINUX_ERROR_NUM);
				iError = TRUE;
			}
			else
			{//shbaek: Done ...
				if(iDBG)GRIB_LOGD("# %s-HTTP-MSG: RECV DONE [TOTAL: %d]\n", httpLogInfo.httpSender, iTotal);
			}
			break;
		}
		else
		{
			MEMCPY(pMsg->recvBuff+iTotal, gLineBuff, iCount);

			iTotal += iCount;
			//if(iDBG)GRIB_LOGD("# RECV: %d\n", iCount);

			if(!STRNCASECMP(gLineBuff, HTTP_TRANS_ENCODE_CHUNK, STRLEN(HTTP_TRANS_ENCODE_CHUNK)))
			{
				isChunk = TRUE;
				if(iDBG)GRIB_LOGD("# TRANSFER ENCODING: CHUNKED !!!\n");
				iCount = Grib_RecvChunked(iFD, pMsg->recvBuff+iTotal, HTTP_MAX_SIZE_RECV_MSG-iTotal, GRIB_NOT_USED);
				iTotal += iCount;
				if(iDBG)GRIB_LOGD("# REAL DATA RECV DONE [TOTAL: %d]\n", iTotal);
				break;
			}
		}
	}while(iTotal < HTTP_MAX_SIZE_RECV_MSG);

FINAL:
	if(iTimeCheck)
	{//2 shbaek: TIME CHECK
		sysTimer = time(NULL);
		sysTime  = localtime(&sysTimer);
		if(iDBG)GRIB_LOGD("# RECV DONE TIME : %02d:%02d:%02d\n", sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec);
	}

	if(0 < iFD)	close(iFD);

	if(iError)
	{
		STRINIT(pMsg->statusMsg, sizeof(pMsg->statusMsg));
//		STRNCPY(pMsg->statusMsg, httpLogInfo.httpErrMsg, STRLEN(httpLogInfo.httpErrMsg));
		STRNCPY(pMsg->statusMsg, LINUX_ERROR_STR, STRLEN(LINUX_ERROR_STR));
		pMsg->statusCode = LINUX_ERROR_NUM;

		Grib_HttpTombStone(&httpLogInfo);
		return GRIB_ERROR;
	}

	return iTotal;
}


