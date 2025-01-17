# coding: utf-8

"""
PathsApi.py
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
"""

from __future__ import absolute_import

import sys
import os

# python 2 and python 3 compatibility library
from six import iteritems

from ..configuration import Configuration
from ..api_client import ApiClient


class PathsApi(object):
    """
    NOTE: This class is auto generated by the swagger code generator program.
    Do not edit the class manually.
    Ref: https://github.com/swagger-api/swagger-codegen
    """

    def __init__(self, api_client=None):
        config = Configuration()
        if api_client:
            self.api_client = api_client
        else:
            if not config.api_client:
                config.api_client = ApiClient()
            self.api_client = config.api_client

    def get_path_info(self, mac_a, mac_b, **kwargs):
        """
        Get information about a path between devices
        

        This method makes a synchronous HTTP request by default. To make an
        asynchronous HTTP request, please define a `callback` function
        to be invoked when receiving the response.
        >>> def callback_function(response):
        >>>     pprint(response)
        >>>
        >>> thread = api.get_path_info(mac_a, mac_b, callback=callback_function)

        :param callback function: The callback function
            for asynchronous request. (optional)
        :param str mac_a: MAC Address of first device (required)
        :param str mac_b: MAC Address of second device (required)
        :return: PathInfo
                 If the method is called asynchronously,
                 returns the request thread.
        """

        all_params = ['mac_a', 'mac_b']
        all_params.append('callback')

        params = locals()
        for key, val in iteritems(params['kwargs']):
            if key not in all_params:
                raise TypeError(
                    "Got an unexpected keyword argument '%s'"
                    " to method get_path_info" % key
                )
            params[key] = val
        del params['kwargs']

        # verify the required parameter 'mac_a' is set
        if ('mac_a' not in params) or (params['mac_a'] is None):
            raise ValueError("Missing the required parameter `mac_a` when calling `get_path_info`")
        # verify the required parameter 'mac_b' is set
        if ('mac_b' not in params) or (params['mac_b'] is None):
            raise ValueError("Missing the required parameter `mac_b` when calling `get_path_info`")

        resource_path = '/paths/p/{macA}/{macB}'.replace('{format}', 'json')
        method = 'GET'

        path_params = {}
        if 'mac_a' in params:
            path_params['macA'] = params['mac_a']
        if 'mac_b' in params:
            path_params['macB'] = params['mac_b']

        query_params = {}

        header_params = {}

        form_params = {}
        files = {}

        body_params = None

        # HTTP header `Accept`
        header_params['Accept'] = self.api_client.\
            select_header_accept(['application/json'])
        if not header_params['Accept']:
            del header_params['Accept']

        # HTTP header `Content-Type`
        header_params['Content-Type'] = self.api_client.\
            select_header_content_type(['application/json'])

        # Authentication setting
        auth_settings = ['dust_basic']

        response = self.api_client.call_api(resource_path, method,
                                            path_params,
                                            query_params,
                                            header_params,
                                            body=body_params,
                                            post_params=form_params,
                                            files=files,
                                            response_type='PathInfo',
                                            auth_settings=auth_settings,
                                            callback=params.get('callback'))
        return response

    def get_connections(self, mac, **kwargs):
        """
        Get the list of connections between a device and its neighbors
        

        This method makes a synchronous HTTP request by default. To make an
        asynchronous HTTP request, please define a `callback` function
        to be invoked when receiving the response.
        >>> def callback_function(response):
        >>>     pprint(response)
        >>>
        >>> thread = api.get_connections(mac, callback=callback_function)

        :param callback function: The callback function
            for asynchronous request. (optional)
        :param str mac: MAC Address (required)
        :param int start_index: Start index for the response elements
        :param int max_results: Maximum number of elements to return
        :param bool details: Provide details for each path
        :return: PathDetailsList
                 If the method is called asynchronously,
                 returns the request thread.
        """

        all_params = ['mac', 'start_index', 'max_results', 'details']
        all_params.append('callback')

        params = locals()
        for key, val in iteritems(params['kwargs']):
            if key not in all_params:
                raise TypeError(
                    "Got an unexpected keyword argument '%s'"
                    " to method get_connections" % key
                )
            params[key] = val
        del params['kwargs']

        # verify the required parameter 'mac' is set
        if ('mac' not in params) or (params['mac'] is None):
            raise ValueError("Missing the required parameter `mac` when calling `get_connections`")

        resource_path = '/paths/p/{mac}'.replace('{format}', 'json')
        method = 'GET'

        path_params = {}
        if 'mac' in params:
            path_params['mac'] = params['mac']

        query_params = {}
        if 'start_index' in params:
            query_params['startIndex'] = params['start_index']
        if 'max_results' in params:
            query_params['maxResults'] = params['max_results']
        if 'details' in params:
            query_params['details'] = params['details']

        header_params = {}

        form_params = {}
        files = {}

        body_params = None

        # HTTP header `Accept`
        header_params['Accept'] = self.api_client.\
            select_header_accept(['application/json'])
        if not header_params['Accept']:
            del header_params['Accept']

        # HTTP header `Content-Type`
        header_params['Content-Type'] = self.api_client.\
            select_header_content_type(['application/json'])

        # Authentication setting
        auth_settings = ['dust_basic']

        response = self.api_client.call_api(resource_path, method,
                                            path_params,
                                            query_params,
                                            header_params,
                                            body=body_params,
                                            post_params=form_params,
                                            files=files,
                                            response_type='PathDetailsList',
                                            auth_settings=auth_settings,
                                            callback=params.get('callback'))
        return response
