/*
 * ds_redis.h
 *
 *  Created on: 2015年10月23日
 *      Author: liwangmijia
 */

//参考数据库结构设计(公共部分)
#ifndef __REDIS_DEFINE_H__
#define __REDIS_DEFINE_H__

// Dssaccess相关信息
#define KEY_DSSACCESSSERVER_MAP                ("RpsMap")

#define KEY_DSSACCESSSERVER_KEY_PREFIX ("Rps_")

#define FIELD_DSSACCESSSERVER_IP                  ("ServerIP")
#define FIELD_DSSACCESSSERVER_PORT                ("ServerPort")
#define FIELD_DSSACCESSSERVER_AREA              ("ServerArea")
#define FIELD_DSSACCESSSERVER_VENDORNAME          ("VendorName")
#define FIELD_DSSACCESSSERVER_STATUS            ("Status")
#define FIELD_DSSACCESSSERVER_ACTIVEINDEX       ("ActiveIndex")
#define FIELD_DSSACCESSSERVER_RETOK			    ("RetOK")
#define FIELD_DSSACCESSSERVER_RUNSECONDS		("RunSeconds")
#define FIELD_DSSACCESSSERVER_RETERROR       ("RetError")

#define VALUE_DSSACCESSSERVER_OFFLINE              ("0")
#define VALUE_DSSACCESSSERVER_ONLINE              ("1")

#endif //__REDIS_WRAP_H__

