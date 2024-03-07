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


class ConnectedNeighbor(object):
    """
    NOTE: This class is auto generated by the swagger code generator program.
    Do not edit the class manually.
    """
    def __init__(self):
        """
        ConnectedNeighbor - a model defined in Swagger

        :param dict swaggerTypes: The key is attribute name
                                  and the value is attribute type.
        :param dict attributeMap: The key is attribute name
                                  and the value is json key in definition.
        """
        self.swagger_types = {
            'mac_address': 'str',
            'num_rx': 'int',
            'num_tx': 'int',
            'num_tx_fail': 'int',
            'rssi': 'int'
        }

        self.attribute_map = {
            'mac_address': 'macAddress',
            'num_rx': 'numRx',
            'num_tx': 'numTx',
            'num_tx_fail': 'numTxFail',
            'rssi': 'rssi'
        }

        self._mac_address = None
        self._num_rx = None
        self._num_tx = None
        self._num_tx_fail = None
        self._rssi = None

    @property
    def mac_address(self):
        """
        Gets the mac_address of this ConnectedNeighbor.
        MAC address of the neighbor

        :return: The mac_address of this ConnectedNeighbor.
        :rtype: str
        """
        return self._mac_address

    @mac_address.setter
    def mac_address(self, mac_address):
        """
        Sets the mac_address of this ConnectedNeighbor.
        MAC address of the neighbor

        :param mac_address: The mac_address of this ConnectedNeighbor.
        :type: str
        """
        self._mac_address = mac_address

    @property
    def num_rx(self):
        """
        Gets the num_rx of this ConnectedNeighbor.
        Number of received packets

        :return: The num_rx of this ConnectedNeighbor.
        :rtype: int
        """
        return self._num_rx

    @num_rx.setter
    def num_rx(self, num_rx):
        """
        Sets the num_rx of this ConnectedNeighbor.
        Number of received packets

        :param num_rx: The num_rx of this ConnectedNeighbor.
        :type: int
        """
        self._num_rx = num_rx

    @property
    def num_tx(self):
        """
        Gets the num_tx of this ConnectedNeighbor.
        Number of transmitted packets

        :return: The num_tx of this ConnectedNeighbor.
        :rtype: int
        """
        return self._num_tx

    @num_tx.setter
    def num_tx(self, num_tx):
        """
        Sets the num_tx of this ConnectedNeighbor.
        Number of transmitted packets

        :param num_tx: The num_tx of this ConnectedNeighbor.
        :type: int
        """
        self._num_tx = num_tx

    @property
    def num_tx_fail(self):
        """
        Gets the num_tx_fail of this ConnectedNeighbor.
        Number of failed transmission attempts

        :return: The num_tx_fail of this ConnectedNeighbor.
        :rtype: int
        """
        return self._num_tx_fail

    @num_tx_fail.setter
    def num_tx_fail(self, num_tx_fail):
        """
        Sets the num_tx_fail of this ConnectedNeighbor.
        Number of failed transmission attempts

        :param num_tx_fail: The num_tx_fail of this ConnectedNeighbor.
        :type: int
        """
        self._num_tx_fail = num_tx_fail

    @property
    def rssi(self):
        """
        Gets the rssi of this ConnectedNeighbor.
        Last reported RSSI

        :return: The rssi of this ConnectedNeighbor.
        :rtype: int
        """
        return self._rssi

    @rssi.setter
    def rssi(self, rssi):
        """
        Sets the rssi of this ConnectedNeighbor.
        Last reported RSSI

        :param rssi: The rssi of this ConnectedNeighbor.
        :type: int
        """
        self._rssi = rssi

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

