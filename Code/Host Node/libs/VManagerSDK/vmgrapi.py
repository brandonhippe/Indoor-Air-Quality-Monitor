'''Helper classes for the VManager Python API

'''

from . import vmanager
import threading
import logging
from .vmanager import notif_client
from queue import Queue, Empty

DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 8888
DEFAULT_TIMEOUT = 1.0 # seconds


class NotifHandler(threading.Thread):
    '''Handle notifications from the queue'''
    def __init__(self, queue, notif_callback=None, disconnect_callback=None):
        threading.Thread.__init__(self, target=self.run)
        self.daemon = True
        self.queue = queue
        self.done = False
        self.timeout = DEFAULT_TIMEOUT
        self.notif_callback = notif_callback
        self.disconnect_callback = disconnect_callback

    def stop(self):
        self.done = True
        self.queue.put(notif_client.QueueFinished())

    def run(self):
        while not self.done:
            try:
                notif_obj = self.queue.get(timeout=self.timeout)
                # print notif_obj
                # queueFinished can be generated by stop() or 
                # by the notification connection closing
                if notif_obj.type == 'queueFinished':
                    break
                elif self.notif_callback:
                    self.notif_callback(notif_obj)
            except Empty:
                pass
        if self.disconnect_callback:
            self.disconnect_callback()

class VManagerApi:
    def __init__(self, host=DEFAULT_HOST, port=DEFAULT_PORT):
        self.url        = 'https://{0}:{1}/manager/v1'.format(host, port)
        self.client     = vmanager.ApiClient(self.url)
        self.usersApi   = vmanager.UsersApi(self.client)
        self.systemApi  = vmanager.SystemApi(self.client)
        self.networkApi = vmanager.NetworkApi(self.client)
        self.alarmsApi  = vmanager.AlarmsApi(self.client)
        self.dclApi     = vmanager.DCLApi(self.client)   
        self.aclApi     = vmanager.ACLApi(self.client)   
        self.configApi  = vmanager.ConfigApi(self.client)
        self.motesApi   = vmanager.MotesApi (self.client)
        self.apApi      = vmanager.APApi (self.client)
        self.pathsApi   = vmanager.PathsApi (self.client)
        self.softwareApi= vmanager.SoftwareApi (self.client)
        self.notifApi   = notif_client.NotifClient(self.url, self.client)
        self.notif_handler = None
        self.notif_connection = None

    def get_notifications(self, notif_filter=None, notif_callback=None, 
                          disconnect_callback=None):
        self.notif_connection = self.notifApi.get_notifications(notif_filter=notif_filter)
        self.notif_handler = NotifHandler(self.notifApi.queue, notif_callback, 
                                          disconnect_callback)
        self.notif_handler.start()

    def stop_notifications(self):
        if self.notif_handler:
            self.notif_handler.stop()
            self.notif_handler.join()
            self.notif_connection.stop()


# sample function for parsing OAP notifications, requires OAPNotif and OAPMessage
# from SmartMeshSDK/protocols/oap

# import OAPNotif
# import OAPMessage
#
# def parse_data(self, data_obj):
#     payload = array('B', base64.b64decode(data_obj.payload))
#     # not sure what the first two bytes of payload are -- OAP message starts at byte 2
#     if payload[2] == OAPMessage.CmdType.NOTIF:
#         oap_notif = OAPNotif.parse_oap_notif(payload, 3) # start at byte 3
#         payload_str = str(oap_notif)
#     else:
#         payload_str = ' '.join('%02X' % b for b in payload)
#
#     time = data_obj.gen_net_time.strftime("%m-%d %H:%M:%S")
#     print data_obj.mac_address, time, oap_notif
