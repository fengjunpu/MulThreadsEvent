#ifndef _H_COMMONDATA_H_
#define _H_COMMONDATA_H_
#include "../src/easylogging++.h"

#define HTTP_RES_200           200
#define HTTP_RES_CREATED       201
#define HTTP_RES_ACCEPTED      202
#define HTTP_RES_NAUTHINFO     203
#define HTTP_RES_NOCONTENT     204
#define HTTP_RES_RSTCONTENT    205
#define HTTP_RES_PARTIAL       206
#define HTTP_RES_MSTATUS       207
#define HTTP_RES_IMUSED        226

#define HTTP_RES_300           300
#define HTTP_RES_MCHOICE       300
#define HTTP_RES_MOVEDPERM     301
#define HTTP_RES_FOUND         302
#define HTTP_RES_SEEOTHER      303
#define HTTP_RES_NOTMOD        304
#define HTTP_RES_USEPROXY      305
#define HTTP_RES_SWITCHPROXY   306
#define HTTP_RES_TMPREDIR      307

#define HTTP_RES_400           400
#define HTTP_RES_BADREQ        400
#define HTTP_RES_UNAUTH        401
#define HTTP_RES_PAYREQ        402
#define HTTP_RES_FORBIDDEN     403
#define HTTP_RES_NOTFOUND      404
#define HTTP_RES_METHNALLOWED  405
#define HTTP_RES_NACCEPTABLE   406
#define HTTP_RES_PROXYAUTHREQ  407
#define HTTP_RES_TIMEOUT       408
#define HTTP_RES_CONFLICT      409
#define HTTP_RES_GONE          410
#define HTTP_RES_LENREQ        411
#define HTTP_RES_PRECONDFAIL   412
#define HTTP_RES_ENTOOLARGE    413
#define HTTP_RES_URITOOLARGE   414
#define HTTP_RES_UNSUPPORTED   415
#define HTTP_RES_RANGENOTSC    416
#define HTTP_RES_EXPECTFAIL    417
#define HTTP_RES_IAMATEAPOT    418

#define HTTP_RES_500           500
#define HTTP_RES_SERVERR       500
#define HTTP_RES_NOTIMPL       501
#define HTTP_RES_BADGATEWAY    502
#define HTTP_RES_SERVUNAVAIL   503
#define HTTP_RES_GWTIMEOUT     504
#define HTTP_RES_VERNSUPPORT   505
#define HTTP_RES_BWEXEED       509


extern char RPS_SERVER_IP[48];	//rpsaccess 服务器的ip
extern int RPS_SERVER_PORT;		//RPS 服务器的PORT
extern char REDIS_CENTER_IP[48];		//数据中心IP
extern int REDIS_STATUS_PORT;	//状态数据库端口
extern int REDIS_AUTH_PORT;		//授权数据库端口
extern int REDIS_RECONN_INTERNAL;	//Redis重连间隔
extern int HEATER_BEAT_INTERNAL;	//设备心跳间隔时间
extern int HEART_BEAT_TIMEOUT;		//心跳超时时间
extern int REDIS_EXPIRE_TIME;		//数据库中元素的过期时间
extern int REDIS_CHECKHEALTH_INTERNAL; //检查数据库连接状态的时间间隔
const char *status_code_to_str(int code);

#endif
