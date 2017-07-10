#ifndef __LIB_MATCH_H__
#define __LIB_MATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

// 从各个redis数据库拉取状态信息，上层需定时调用
// 输入：redis_ip: 各数据中心的redis ip，以冒号分隔，比如: "1.1.1.1:2.2.2.2"
//建议3分钟调用一次
// 返回值：>= 0 成功， < 0 失败
int refresh_server_info(char *redis_ip);

// 根据oem、区域信息、负载情况获取最优服务器
// 输入
//      in_redis: 本数据中心的redis_ip
//      service：服务类型 （可取值 P2P，DSS，DSSPub，DSSStream，PMS，TPS, CSS，ALC ,PIC，FDFS(指tracker)，AUTH, STATUS）
//      oem：		 OEM信息	（可取值 "General"，"定制ID"）
//      area：	 区域信息	（可取值 洲:国家:省，如"Asia:China:Beijing"）
// 输出
//      matched_server: 匹配的IP地址，理论上必须有返回
//      matched_redis: 设备所属的数据中心的redis IP地址，理论上必须有返回
// 返回值：>= 0 成功， < 0 失败
int get_matched_server(char *in_redis, char *service,char *oem, char *area, char * matched_server, char *matched_redis);

#ifdef __cplusplus
}
#endif

#endif

