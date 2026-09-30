typedef int      LGNC_STATUS_T;
typedef int      BOOLEAN;
typedef unsigned int   UINT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef int      SINT32;
typedef int      LGNC_KEY_COND_T;      //0=PRESS, 1=RELEASE, 2=REPEAT, 3=DRAG, 4=POWER, 5=GESTURE
typedef void     LGNC_ADDITIONAL_INPUT_INFO_T; //unk, use void

#define LGNC_HANDLED 0

typedef LGNC_STATUS_T (*LGNC_MSG_HANDLER_T)(int msgType, UINT32, UINT8*, UINT16);
typedef BOOLEAN (*LGNC_KEY_CALLBACK_T)(UINT32 keyCode, LGNC_KEY_COND_T cond, LGNC_ADDITIONAL_INPUT_INFO_T *extra);
typedef BOOLEAN (*LGNC_MOUSE_CALLBACK_T)(SINT32, SINT32, UINT32, LGNC_KEY_COND_T, LGNC_ADDITIONAL_INPUT_INFO_T*);

typedef struct {
	LGNC_MSG_HANDLER_T     pfnMsgHandler;
	LGNC_KEY_CALLBACK_T    pfnKeyEventCallback;
	LGNC_MOUSE_CALLBACK_T  pfnMouseEventCallback;
	void                  *pfnJoystickEventCallback;
} LGNC_SYSTEM_CALLBACKS_T;

extern LGNC_STATUS_T LGNC_SYSTEM_Initialize(int argc, char **argv, LGNC_SYSTEM_CALLBACKS_T *pCallbacks);
extern void LGNC_SYSTEM_Finalize(void);

//key codes 
#define KEY_UP     0x40
#define KEY_DOWN   0x41
#define KEY_LEFT   0x07
#define KEY_RIGHT  0x06
#define KEY_OK     0x44
#define KEY_BACK   0x28
#define KEY_NUM0   0x10 
//0x10-0x19 numeric keys 0-9
#define KEY_NUM9   0x19 
#define KEY_VOLUP  0x02
#define KEY_VOLDN  0x03
#define KEY_MUTE   0x09

//messge types (LGNC_MSG_TYPE_T)
#define LGNC_MSG_NONE		0
#define LGNC_MSG_FOCUS_IN 	1
#define LGNC_MSG_FOCUS_OUT 	2
#define LGNC_MSG_TERMINATE 	3
#define LGNC_MSG_HOST_EVENT 4