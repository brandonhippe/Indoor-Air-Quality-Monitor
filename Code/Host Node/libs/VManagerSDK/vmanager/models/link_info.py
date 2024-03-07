# coding: utf-8

"""
Copyright 2015 SmartBear Software

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    Ref: https://github.com/swagger-api/swagger-codegen
"""

from pprint import pformat
from six import iteritems


class LinkInfo(object):
    """
    NOTE: This class is auto generated by the swagger code generator program.
    Do not edit the class manually.
    """
    def __init__(self):
        """
        LinkInfo - a model defined in Swagger

        :param dict swaggerTypes: The key is attribute name
                                  and the value is attribute type.
        :param dict attributeMap: The key is attribute name
                                  and the value is json key in definition.
        """
        self.swagger_types = {
            'channel_offset': 'int',
            'frame_id': 'int',
            'mac_address': 'str',
            'properties': 'list[str]',
            'slot': 'int'
        }

        self.attribute_map = {
            'channel_offset': 'channelOffset',
            'frame_id': 'frameId',
            'mac_address': 'macAddress',
            'properties': 'properties',
            'slot': 'slot'
        }

        self._channel_offset = None
        self._frame_id = None
        self._mac_address = None
        self._properties = None
        self._slot = None

    @property
    def channel_offset(self):
        """
        Gets the channel_offset of this LinkInfo.
        Channel offset

        :return: The channel_offset of this LinkInfo.
        :rtype: int
        """
        return self._channel_offset

    @channel_offset.setter
    def channel_offset(self, channel_offset):
        """
        Sets the channel_offset of this LinkInfo.
        Channel offset

        :param channel_offset: The channel_offset of this LinkInfo.
        :type: int
        """
        self._channel_offset = channel_offset

    @property
    def frame_id(self):
        """
        Gets the frame_id of this LinkInfo.
        Frame id

        :return: The frame_id of this LinkInfo.
        :rtype: int
        """
        return self._frame_id

    @frame_id.setter
    def frame_id(self, frame_id):
        """
        Sets the frame_id of this LinkInfo.
        Frame id

        :param frame_id: The frame_id of this LinkInfo.
        :type: int
        """
        self._frame_id = frame_id

    @property
    def mac_address(self):
        """
        Gets the mac_address of this LinkInfo.
        MAC address of the peer device

        :return: The mac_address of this LinkInfo.
        :rtype: str
        """
        return self._mac_address

    @mac_address.setter
    def mac_address(self, mac_address):
        """
        Sets the mac_address of this LinkInfo.
        MAC address of the peer device

        :param mac_address: The mac_address of this LinkInfo.
        :type: str
        """
        self._mac_address = mac_address

    @property
    def properties(self):
        """
        Gets the properties of this LinkInfo.
        A list of properties of the link. Properties include 'tx' (transmission), 'rx' (reception), 'bcast' (broadcast), or 'mcast' (multicast).

        :return: The properties of this LinkInfo.
        :rtype: list[str]
        """
        return self._properties

    @properties.setter
    def properties(self, properties):
        """
        Sets the properties of this LinkInfo.
        A list of properties of the link. Properties include 'tx' (transmission), 'rx' (reception), 'bcast' (broadcast), or 'mcast' (multicast).

        :param properties: The properties of this LinkInfo.
        :type: list[str]
        """
        self._properties = properties

    @property
    def slot(self):
        """
        Gets the slot of this LinkInfo.
        Slot number

        :return: The slot of this LinkInfo.
        :rtype: int
        """
        return self._slot

    @slot.setter
    def slot(self, slot):
        """
        Sets the slot of this LinkInfo.
        Slot number

        :param slot: The slot of this LinkInfo.
        :type: int
        """
        self._slot = slot

    def to_dict(self):
        """
        Returns the model properties as a dict
        """
        result = {}

        for attr, _ in iteritems(self.swagger_types):
            value = getattr(self, attr)
            if isinstance(value, list):
                result[attr] = list(map(
                    lambda x: x.to_dict() if hasattr(x, "to_dict") else x,
                    value
                ))
            elif hasattr(value, "to_dict"):
                result[attr] = value.to_dict()
            else:
                result[attr] = value

        return result

    def to_str(self):
        """
        Returns the string representation of the model
        """
        return pformat(self.to_dict())

    def __repr__(self):
        """
        For `print` and `pprint`
        """
        return self.to_str()

    def __eq__(self, other): 
        """
        Returns true if both objects are equal
        """
        return self.__dict__ == other.__dict__

    def __ne__(self, other):
        """ 
        Returns true if both objects are not equal
        """
        return not self == other

