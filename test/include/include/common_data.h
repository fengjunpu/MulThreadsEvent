#ifndef _H_COMMONDATA_H_
#define _H_COMMONDATA_H_

extern char RPS_SERVER_IP[48];	//rpsaccess ��������ip
extern int RPS_SERVER_PORT;		//RPS ��������PORT
extern char REDIS_CENTER_IP[48];		//��������IP
extern int REDIS_STATUS_PORT;	//״̬���ݿ�˿�
extern int REDIS_AUTH_PORT;		//��Ȩ���ݿ�˿�
extern int REDIS_RECONN_INTERNAL;	//Redis�������
extern int HEATER_BEAT_INTERNAL;	//�豸�������ʱ��
extern int HEART_BEAT_TIMEOUT;		//������ʱʱ��
extern int REDIS_EXPIRE_TIME;		//���ݿ���Ԫ�صĹ���ʱ��
extern int REDIS_CHECKHEALTH_INTERNAL; //������ݿ�����״̬��ʱ����

#endif
