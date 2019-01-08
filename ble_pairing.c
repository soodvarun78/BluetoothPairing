#include<stdio.h>
#include<glib-2.0/glib.h>
#include<gio/gio.h>
#include<string.h>
#include<stdlib.h>

//State Machine
#define IDLE 1
#define CONNECTING 2
#define CONNECTED 3

int gPaired = 0;
GDBusConnection *conn; 
guint rid;
static int current_value = 0;
static GMainLoop *loop = NULL;
char gPstring[10];
GDBusMessage *gPMsg;
gchar pDeviceCan[50]="Invalid";
int context_pair = 0;
int context_connect = 0;
char gConnectAddr[30]; 

char gObjectPath[50];

void iter_object(GVariant *parameters);
void signal_handler_interface_added(GDBusConnection *conn,const gchar *sender_name,const gchar *object_path,const gchar *interface_name,const gchar *signal_name,GVariant *parameters,gpointer user_data);
void signal_handler_properties_changed(GDBusConnection *conn,const gchar *sender_name,const gchar *object_path,const gchar *interface_name,const gchar *signal_name,GVariant *parameters,gpointer user_data);
void iterate_container_recursive1(GVariant *container);
void dbus_set_property(char *obj,char *interface ,char *property, GVariant *value);
GVariant* dbus_get_property(char *obj, char *interface, char *property);
void * MonitorThread(void *arg);
int state = IDLE;
static const GDBusArgInfo arg1_authorizeservice = {
        -1, "device", "o", NULL
};

static const GDBusArgInfo arg2_authorizeservice = {
        -1, "uuid", "s", NULL
};

static const GDBusArgInfo arg1_requestauthorization = {
        -1, "device", "o", NULL
};

static const GDBusArgInfo arg1_requestconfirmation = {
        -1, "device", "o", NULL
};

static const GDBusArgInfo arg2_requestconfirmation = {
        -1, "passkey", "u", NULL
};

static const GDBusArgInfo arg1_displaypasskey = {
        -1, "device", "o", NULL
};

static const GDBusArgInfo arg2_displaypasskey = {
        -1, "passkey", "u", NULL
};

static const GDBusArgInfo arg3_displaypasskey = {
        -1, "entered", "q", NULL
};

static const GDBusArgInfo arg1_requestpasskey = {
        -1, "device", "o", NULL
};
static const GDBusArgInfo arg2_requestpasskey = {
        -1, "key", "u", NULL
};
static const GDBusArgInfo arg1_displaypincode = {
        -1, "object", "o", NULL
};
static const GDBusArgInfo arg2_displaypincode = {
        -1, "pincode", "s", NULL
};

static const GDBusArgInfo arg1_requestpincode = {
        -1, "device", "o", NULL
};
static const GDBusArgInfo arg2_requestpincode = {
        -1, "key", "s", NULL
};
	
static const GDBusArgInfo *in_AuthorizeService[] = {
        &arg1_authorizeservice,
        &arg2_authorizeservice,
	NULL
};
static const GDBusArgInfo *in_RequestAuthorization[] = {
        &arg1_requestauthorization,
	NULL
};
#if 0
static const GDBusArgInfo *in_RequestConfirmation[] = {
        &arg1_requestconfirmation,
	NULL
};
#endif
static const GDBusArgInfo *in_RequestConfirmation[] = {
        &arg1_requestconfirmation,
	&arg2_requestconfirmation,
	NULL
};
static const GDBusArgInfo *in_DisplayPasskey[] = {
        &arg1_displaypasskey,
	&arg2_displaypasskey,
	&arg3_displaypasskey,
	NULL
};

static const GDBusArgInfo *in_RequestPasskey[] = {
	&arg1_requestpasskey,
	NULL
};
static const GDBusArgInfo *out_RequestPasskey[] = {
	&arg2_requestpasskey,
	NULL
};

static const GDBusArgInfo *in_DisplayPinCode[] = {
        &arg1_displaypincode,
	&arg2_displaypincode,
	NULL
};
static const GDBusArgInfo *in_RequestPincode[] = {
	&arg1_requestpincode,
	NULL
};
static const GDBusArgInfo *out_RequestPincode[] = {
	&arg2_requestpincode,
	NULL
};
static const GDBusMethodInfo agent_iface_endpoint_AuthorizeService = {
        -1, "AuthorizeService",
        (GDBusArgInfo **)in_AuthorizeService,
        NULL,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_Cancel = {
        -1, "Cancel",
        NULL,
        NULL,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_RequestAuthorization = {
        -1, "RequestAuthorization",
        (GDBusArgInfo **)in_RequestAuthorization,
        NULL,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_RequestConfirmation = {
        -1, "RequestConfirmation",
        (GDBusArgInfo **)in_RequestConfirmation,
        NULL,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_DisplayPasskey = {
        -1, "DisplayPaskey",
        (GDBusArgInfo **)in_DisplayPasskey,
        NULL,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_RequestPasskey = {
        -1, "RequestPasskey",
        (GDBusArgInfo **)in_RequestPasskey,
        (GDBusArgInfo **)out_RequestPasskey,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_DisplayPinCode = {
        -1, "DisplayPinCode",
        (GDBusArgInfo **)in_DisplayPinCode,
        NULL,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_RequestPinCode = {
        -1, "RequestPasskey",
        (GDBusArgInfo **)in_RequestPincode,
        (GDBusArgInfo **)out_RequestPincode,
        NULL,
};
static const GDBusMethodInfo agent_iface_endpoint_Release = {
        -1, "Release",
        NULL,
        NULL,
        NULL,
};
static const GDBusMethodInfo *agent_iface_endpoint_methods[] = {
        &agent_iface_endpoint_AuthorizeService,
        &agent_iface_endpoint_Cancel,
	&agent_iface_endpoint_RequestAuthorization,
	&agent_iface_endpoint_RequestConfirmation,
	&agent_iface_endpoint_DisplayPasskey,
	&agent_iface_endpoint_RequestPasskey,
	&agent_iface_endpoint_DisplayPinCode,
	&agent_iface_endpoint_RequestPinCode,
	&agent_iface_endpoint_Release,
};

const GDBusInterfaceInfo agent_iface_endpoint = {
        -1, "org.bluez.Agent1",
        (GDBusMethodInfo **)agent_iface_endpoint_methods,
        NULL,
        NULL,
        NULL,
};      
/* Play call bluealsa-aplay <BT Address> */
void play_bluealsa () {
	char temp1[40];
	strncpy(temp1,"bluealsa-aplay ",15); //bluealsa-aplay size
	strncat(temp1,gConnectAddr,17); //MAC address
	strcat(temp1," &");//Run in backgroud 
	printf("temp1 %s \n",temp1);
	system(temp1);
	g_main_loop_quit (loop);

}
/* Wait for 10 sec for connection to be confirmed */
int gotdevice = 0;
void * ConnectThread(void *arg) {
	GVariant *value;
	sleep(10);
	printf("gObjectPAth %s \n",gObjectPath);
	value = dbus_get_property(gObjectPath,"org.bluez.Device1","Connected");
	printf("boolean is %d \n",g_variant_get_boolean(value));
	if(g_variant_get_boolean(value) == 1){
		play_bluealsa ();
		state = CONNECTED;	
	}
	else {
		state = IDLE;
	}
}
/* Parse the data in the signal handlers */
void iterate_container_recursive1(GVariant *container){
  GVariantIter iter;
  GVariant *child;
  const gchar *objstring = NULL;
  gboolean paired=0;
  gboolean connect=0;
  const gchar *pstring = NULL;
  pthread_t Thread2;

  g_variant_iter_init (&iter, container);
  while ((child = g_variant_iter_next_value (&iter)))
    {
      g_print ("type '%s'\n", g_variant_get_type_string (child));
      if(!strcmp(g_variant_get_type_string (child),"o")){
	if(objstring == NULL)
		objstring = g_variant_get_string(child,NULL);
		printf("objstring %s \n",objstring);
	if(!strstr(objstring,"dev_")){ 
		objstring = NULL;
	}
		//dbus_get_property(objstring,"org.bluez.Device1","Paired",0);
      }
      if(context_connect == 1) {
      		if(!strcmp(g_variant_get_type_string (child),"s")) {
			
			if(!strcmp("org.bluez.Device1",g_variant_get_string(child,NULL))){
				printf("Got Device Interface \n");	
				gotdevice =1 ;
			}
			pstring = g_variant_get_string(child,NULL);
			printf("pstring %s \n",pstring);
			if(!strcmp(pstring,"Connected"))
				strcpy(gPstring,pstring); 
	
      		}
		if(!strcmp(g_variant_get_type_string (child),"b")){
			if(!strcmp(gPstring,"Connected") && (gotdevice == 1)){
				connect = g_variant_get_boolean(child);
				printf("connect is %d \n",connect);
				if(connect == 1){
					//play_bluealsa();
					state = CONNECTING;
					if(pthread_create(&Thread2,NULL,ConnectThread,NULL)<0) {
                				printf("pthread_create returned error \n");
        				}
				}
			}
		}
      }
      if(context_pair == 1) {
      		if(!strcmp(g_variant_get_type_string (child),"s")){
      			if(!strcmp("org.bluez.Device1",g_variant_get_string(child,NULL))||
				!strcmp("org.bluez.MediaControl1",g_variant_get_string(child,NULL))){
				printf("Got Device Interface \n");	
				gotdevice =1 ;
			}
			memset(gPstring,0,sizeof(gPstring));
			pstring = g_variant_get_string(child,NULL);
			printf("pstring -- [%s] \n",pstring);
			if(!strcmp(pstring,"Paired")||!strcmp(pstring,"Connected"))
				strcpy(gPstring,pstring); 
			//if(!strcmp(pstring,"Paired") && !strcmp(pstring,"Connected"))
			//	pstring = NULL;
		//goto next;
      	}
      	if(!strcmp(g_variant_get_type_string (child),"b")){
      		if(!strcmp(gPstring,"Paired")&& (gotdevice == 1)){
			paired = g_variant_get_boolean(child);
			if(paired == 1)
				current_value++;
      		}
     		if(!strcmp(gPstring,"Connected")&& (gotdevice == 1)){
			connect = g_variant_get_boolean(child);
			printf("connect is %d \n",connect);
			if(connect == 1){
      				current_value++;
				state = CONNECTING;
				if(pthread_create(&Thread2,NULL,ConnectThread,NULL)<0) {
                			printf("pthread_create returned error \n");
        			}

			}
		}
      	}
      }
      if (g_variant_is_container (child))
        	iterate_container_recursive1 (child);

      g_variant_unref (child);
    }
}

/* Handle agent calls .. Just returning doing nothing */
static void agent_endpoint_method_call(GDBusConnection *conn, const gchar *sender,
                const gchar *path, const gchar *interface, const gchar *method, GVariant *params,
                GDBusMethodInvocation *invocation, void *userdata) {
	GDBusMessage *reply;
	GDBusMessage *msg;
	GError *error = NULL;
	GVariant *value;
        (void)conn;
        (void)sender;
        (void)interface;
        (void)params;
	printf("%s \n",interface);
	printf(" &&&&&&&&&&&&&&&&&&&&& %s &&&&&&&&&&&&&&&&\n",method);
	msg = g_dbus_method_invocation_get_message (invocation);
	reply = g_dbus_message_new_method_reply (msg);

	if(!strcmp(method,"Release"))
		;
	else if(!strcmp(method,"RequestPinCode")){
		gchar *device;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o)",&device);	
		printf("device addr [%s] \n",device);
        	g_dbus_message_set_body(reply, g_variant_new("(s)","ABCD1234"));
	}
	else if(!strcmp(method,"DisplayPincode")) {
		gchar *device,*pincode;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o&s)",&device,&pincode);	
		printf("device addr [%s] pincode [%s]\n",device,pincode);
	}
	else if(!strcmp(method,"RequestPassKey")){
		gchar *device;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o)",&device);	
        	g_dbus_message_set_body(msg, g_variant_new("(u)",123456));
	}
	else if(!strcmp(method,"DisplayPasskey")) {
		gchar *device;
		guint32	passkey;
		guint16 entered;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o&u&q)",&device,&passkey,&entered);	
		printf("device addr [%s] passkey [%d] entered [%d]\n",device,passkey,entered);
	}
	else if(!strcmp(method,"RequestConfirmation")){
		gchar *device;
		guint32 passkey;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o&u)",&device,&passkey);	
		printf("device addr [%s] pincode [%d]\n",device,passkey);
	}
	else if(!strcmp(method,"RequestAuthorization")){
		gchar *device;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o)",&device);	
		printf("device addr [%s] \n",device);
	}
	else if(!strcmp(method,"AuthorizeService")){
		gchar *device,*uuid;
		value = g_dbus_method_invocation_get_parameters(invocation);
		g_variant_get(value,"(&o&s)",&device,&uuid);	
		printf("device addr [%s] uuid [%s]\n",device,uuid);
		strcpy(pDeviceCan,device);
	}
	else if(!strcmp(method,"Cancel"))
		;
#if 0 
	if (!strcmp(method, "AuthorizeService") || !strcmp(method, "RequestConfirmation") || !strcmp(method,"RequestAuthorization")){
		printf("sender %s \n",sender);
		printf("interface %s \n",interface);
		printf("---%s \n",g_dbus_method_invocation_get_method_name(invocation));
		if(!strcmp(method,"RequestAuthorization")) {
			value = g_dbus_method_invocation_get_parameters(invocation);
			g_print ("type '%s'\n", g_variant_get_type_string (value));
			g_variant_get(value,"(&o)",&device);	
			printf("device addr [%s] \n",device);
		}
#endif
		if(g_dbus_connection_send_message (conn,reply,G_DBUS_SEND_MESSAGE_FLAGS_NONE,NULL, /* out_serial */&error) == 0){
			printf("returned error g_dbus_connection_send_message \n");	
			printf("error %s \n",error?error->message:NULL);
		}
#if 0
	}
	else if (strcmp(method, "Cancel") == 0){
		printf("Cancel called \n");
	}
#endif
	 g_object_unref(invocation);
	return ;
}

static const GDBusInterfaceVTable endpoint_vtable = {
        .method_call = agent_endpoint_method_call,
};
static void endpoint_free(gpointer data) {
        (void)data;
}

void dbus_set_property(char *obj ,char *interface ,char *property, GVariant *value) {
	GDBusMessage *rep;		
	GError *err = NULL;
	GDBusMessage *msg = g_dbus_message_new_method_call("org.bluez",/*"/org/bluez/hci0"*/obj,"org.freedesktop.DBus.Properties","Set");
        g_dbus_message_set_body(msg, g_variant_new("(ssv)",/*"org.bluez.Adapter1"*/interface,property,value));

	if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, msg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL) {
		printf("rep in NULL \n");
	}
	printf("return msg type  %d \n",g_dbus_message_get_message_type(rep));
        if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
                g_dbus_message_to_gerror(rep, &err);
		printf("error is %s \n",err->message);	
		
	}
}

GVariant* dbus_get_property(char *obj, char *interface, char *property) {
	GDBusMessage *rep;		
	GError *err = NULL;
	GVariant *value;
	gPMsg = g_dbus_message_new_method_call("org.bluez",obj,"org.freedesktop.DBus.Properties","Get");
        g_dbus_message_set_body(gPMsg, g_variant_new("(ss)",interface,property));
	if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, gPMsg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL){
		printf("g_dbus_connection_send_message_with_reply_sync failed \n");
	}
        if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
                g_dbus_message_to_gerror(rep, &err);
		printf("g_dbus_connection_send_message_with_reply_sync failed \n");
		printf("error is %s \n",err->message);	
	}
	value = g_dbus_message_get_body(rep), 
	printf("variant type %s \n",g_variant_get_type_string(value));
	g_variant_get(value,"(v)", &value);
	return value;
}

void convert_address(char * addr) {
	char *temp;
	int i =0; 
	memset(gConnectAddr,0,sizeof(gConnectAddr));
	if(addr != NULL) {
		strcpy(gConnectAddr,addr);
		printf("gDevice_Addr %s \n",gConnectAddr);
			temp = gConnectAddr + 4;
			while(temp[i] != '\0'){
				if(temp[i] == '_')
					temp[i]=':';
				i++;
			}
	}
	printf("temp --- %s \n",temp);
	//memset(gConnectAddr,0,sizeof(gConnectAddr));
	strcpy(gConnectAddr,temp);
	//for(i=0;i<30;i++)
	//	printf("%c \n",gConnectAddr[i]);
	printf("gConnectAddr %s \n",gConnectAddr);
}
/* For signal handler interface added */
void signal_handler_interface_added(GDBusConnection *conn,const gchar *sender_name,const gchar *object_path,const gchar *interface_name,const gchar *signal_name,GVariant *parameters,gpointer user_data){
		printf("%s \n",__FUNCTION__);
		//printf("sender_name %s \n",sender_name);
		//printf("signal_name %s \n",signal_name);
		//printf("obj_path %s \n",object_path);
		//printf("interface_name %s \n",interface_name);
		printf("obj_path [%s] interface name [%s] signal_name [%s] \n",object_path,interface_name,signal_name);
		printf("variant type %s \n",g_variant_get_type_string(parameters));
		if(!strcmp(signal_name,"InterfacesAdded") && !strcmp(interface_name,"org.freedesktop.DBus.ObjectManager")){
			if(strstr(object_path,"/dev_") != NULL)
				iterate_container_recursive1(parameters);
		}
		//iterate_container_recursive(parameters);
}

/* For signal handler properties changed */
void signal_handler_properties_changed(GDBusConnection *conn,const gchar *sender_name,const gchar *object_path,const gchar *interface_name,const gchar *signal_name,GVariant *parameters,gpointer user_data){
		char *path;
		printf("%s \n",__FUNCTION__);
		//printf("sender_name %s \n",sender_name);
		//printf("obj_path %s \n",object_path);
		//printf("interface_name %s \n",interface_name);
		printf("obj_path [%s] interface name [%s] signal_name [%s] \n",object_path,interface_name,signal_name);
		printf("variant type %s \n",g_variant_get_type_string(parameters));
		if(!strcmp(signal_name,"PropertiesChanged") && !strcmp(interface_name,"org.freedesktop.DBus.Properties")){
			if(strstr(object_path,"/dev_") != NULL){
				if(((context_connect == 1) || (context_pair == 1))&& (state == IDLE)) {
					//save object path
					memset(gObjectPath,0,50);
					strcpy(gObjectPath,object_path);
					path = strstr(object_path,"dev_");
					convert_address(path);
				}
				gotdevice = 0;
				iterate_container_recursive1(parameters);
			}
		}
}

/* Monitor Pairing for 120 seconds */
void * MonitorThread(void *arg) {
	int count=0; //seconds elapsed 
	while(count < 120) {
		printf("count %d \n",count);
		count++;
		sleep(1);
	}
	g_main_loop_quit (loop);
	return NULL;
}

int main(int argc, char *argv[]) {
	GError *err = NULL;
	GVariant *value;
	GDBusMessage *msg;
	GDBusMessage *rep;		
	pthread_t Thread1;
	int sig_ret;
	gchar* address;
	if(argc != 2) {
		printf("Usage: ./ble_pairing <pair>|<connect> \n");
		return -1;
	}
	//printf("[%s] [%s] \n",argv[0],argv[1]);

	if(!strcmp(argv[1],"pair")) //The Remote device is being paired with local device
		context_pair = 1;
	else if(!strcmp(argv[1],"connect"))  //The Remote device is already paired with local device. It is being connected
		context_connect = 1;
	else {
		printf("Invalid option %s \n",argv[1]);
		return -1;
	}

	address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	printf("address %s \n",address);	

	conn =	g_dbus_connection_new_for_address_sync(address,
                                        G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                        G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                        NULL, NULL, &err); 
	if(conn == NULL){
		printf("err %p \n",err);
                printf("Couldn't obtain D-Bus connection: %s\n",err?err->message:"NULL");
                return -1;
        }

/* Enable Power, Discoverable and Pairable of Adapter */

	value = g_variant_new_boolean(1);
	dbus_set_property("/org/bluez/hci0","org.bluez.Adapter1","Powered",value);

	value = g_variant_new_boolean(1);
	dbus_set_property("/org/bluez/hci0","org.bluez.Adapter1","Discoverable",value);

	if(context_pair == 1) {
		value = g_variant_new_boolean(1);
		dbus_set_property("/org/bluez/hci0","org.bluez.Adapter1","Pairable",value);
	}
/* Print whether adapter is pairable, discoverble and Powered Up */
	value = dbus_get_property("/org/bluez/hci0","org.bluez.Adapter1","Powered");
	printf("Powered is  [%d] \n",g_variant_get_boolean(value));
	g_object_unref(gPMsg);
	value = dbus_get_property("/org/bluez/hci0","org.bluez.Adapter1","Discoverable");
	printf("Discoverable is  [%d] \n",g_variant_get_boolean(value));
	g_object_unref(gPMsg);
	if(context_pair == 1) {
		value = dbus_get_property("/org/bluez/hci0","org.bluez.Adapter1","Pairable");
		printf("Paired is  [%d] \n",g_variant_get_boolean(value));
		g_object_unref(gPMsg);
	}

/*Registering default agent and telling bluez it does not have keyboard and Display */ 
	if((context_pair == 1) ||(context_connect == 1)) {
		msg = g_dbus_message_new_method_call("org.bluez","/org/bluez","org.bluez.AgentManager1","RegisterAgent");

        	g_dbus_message_set_body(msg, g_variant_new("(os)","/org/bluez/agent",/*"KeyboardDisplay"*/"NoInputNoOutput"));
		if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, msg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL){
			printf("g_dbus_connection_send_message_with_reply_sync failed \n");
		}
        	if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
                	g_dbus_message_to_gerror(rep, &err);
			printf("g_dbus_connection_send_message_with_reply_sync failed \n");
			printf("error is %s \n",err->message);
		}

		g_object_unref(msg);

		msg = g_dbus_message_new_method_call("org.bluez","/org/bluez","org.bluez.AgentManager1","RequestDefaultAgent");
        	g_dbus_message_set_body(msg, g_variant_new("(o)","/org/bluez/agent"));
		if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, msg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL){
			printf("g_dbus_connection_send_message_with_reply_sync failed \n");
		}
        	if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
                	g_dbus_message_to_gerror(rep, &err);
			printf("g_dbus_connection_send_message_with_reply_sync failed \n");
		printf("error is %s \n",err->message);
		}
		g_object_unref(msg);
		if ((rid = g_dbus_connection_register_object(conn,"/org/bluez/agent" ,
                                        (GDBusInterfaceInfo *)&agent_iface_endpoint, &endpoint_vtable,
                                        NULL, endpoint_free, &err)) == 0){
	
			printf("g_dbus_connection_register_object returned error \n");
			printf("error is %s \n",err->message);
		}
	}
/* Register Signal handlers one for InterfaceAdded and InterfaceChanged */
	sig_ret = g_dbus_connection_signal_subscribe (conn,"org.bluez",/*NULL*/"org.freedesktop.DBus.ObjectManager",/*NULL*/"InterfacesAdded",/*"/org/bluez"*/NULL,NULL,G_DBUS_SIGNAL_FLAGS_NONE,signal_handler_interface_added,NULL,NULL);
	printf("sig_ret returned %d \n",sig_ret);
	sig_ret = g_dbus_connection_signal_subscribe (conn,"org.bluez",/*NULL*/"org.freedesktop.DBus.Properties",/*NULL*/"PropertiesChanged",/*"/org/bluez/hci0/dev_E4_0E_EE_27_E4_A8"*/NULL,NULL,G_DBUS_SIGNAL_FLAGS_NONE,signal_handler_properties_changed,NULL,NULL);
	printf("sig_ret returned %d \n",sig_ret);
#if 0
	msg = g_dbus_message_new_method_call("org.bluez","/org/bluez/hci0","org.bluez.Adapter1","StartDiscovery");
	if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, msg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL){
		printf("g_dbus_connection_send_message_with_reply_sync failed for StartDiscovery\n");
	}
	g_object_unref(msg);
#endif
	if(context_pair == 1){
		if(pthread_create(&Thread1,NULL,MonitorThread,NULL)<0) {
                	printf("pthread_create returned error \n");
        	}
	}
	loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(loop); //Run main loop
#if 0
	if(context_pair == 1) {
		if((current_value == 2) && (strcmp(pDeviceCan,"Invalid"))!= 0){
			char *temp; //Get MAC address of BLE speaker
			char temp1[40];
			char *addr = strstr(pDeviceCan,"dev_");
			char gDeviceAddr[25];
			int i=0;
			memset(temp1,0,40);
			printf("device path [%s] \n",pDeviceCan);
			dbus_set_property(pDeviceCan,"org.bluez.Device1","Trusted",g_variant_new_boolean(1));
	
			if(addr != NULL) {
				strcpy(gDeviceAddr,addr);
				printf("gDevice_Addr %s \n",gDeviceAddr);
				temp = gDeviceAddr + 4;
				while(temp[i] != '\0'){
					if(temp[i] == '_')
						temp[i]=':';
					i++;
				}
	    		}
			printf("temp ---------- %s \n",temp);
			strncpy(temp1,"bluealsa-aplay ",15); //bluealsa-aplay size
			strncat(temp1,temp,17); //MAC address
			strcat(temp1," &");//Run in backgroud 
			printf("temp1 %s \n",temp1);
	    		system(temp1);
			printf(" +++ ----------- ++++++ temp1 %s \n",temp1);
	    	//while(1)
		//	sleep(1);	
		}	
	}
#endif
}	
