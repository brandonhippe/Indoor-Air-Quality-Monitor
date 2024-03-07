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


class SoftwareInfo(object):
    """
    NOTE: This class is auto generated by the swagger code generator program.
    Do not edit the class manually.
    """
    def __init__(self):
        """
        SoftwareInfo - a model defined in Swagger

        :param dict swaggerTypes: The key is attribute name
                                  and the value is attribute type.
        :param dict attributeMap: The key is attribute name
                                  and the value is json key in definition.
        """
        self.swagger_types = {
            'component': 'str',
            'label': 'str',
            'sw_rev': 'str'
        }

        self.attribute_map = {
            'component': 'component',
            'label': 'label',
            'sw_rev': 'swRev'
        }

        self._component = None
        self._label = None
        self._sw_rev = None

    @property
    def component(self):
        """
        Gets the component of this SoftwareInfo.
        String identifier of the component

        :return: The component of this SoftwareInfo.
        :rtype: str
        """
        return self._component

    @component.setter
    def component(self, component):
        """
        Sets the component of this SoftwareInfo.
        String identifier of the component

        :param component: The component of this SoftwareInfo.
        :type: str
        """
        self._component = component

    @property
    def label(self):
        """
        Gets the label of this SoftwareInfo.
        Version label of the component. The label may contain additional information about  the component, but the format is not specified.

        :return: The label of this SoftwareInfo.
        :rtype: str
        """
        return self._label

    @label.setter
    def label(self, label):
        """
        Sets the label of this SoftwareInfo.
        Version label of the component. The label may contain additional information about  the component, but the format is not specified.

        :param label: The label of this SoftwareInfo.
        :type: str
        """
        self._label = label

    @property
    def sw_rev(self):
        """
        Gets the sw_rev of this SoftwareInfo.
        Software version of the component

        :return: The sw_rev of this SoftwareInfo.
        :rtype: str
        """
        return self._sw_rev

    @sw_rev.setter
    def sw_rev(self, sw_rev):
        """
        Sets the sw_rev of this SoftwareInfo.
        Software version of the component

        :param sw_rev: The sw_rev of this SoftwareInfo.
        :type: str
        """
        self._sw_rev = sw_rev

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

