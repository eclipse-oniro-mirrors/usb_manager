/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "usbd_client.h"
#include <sstream>
#include "hilog_wrapper.h"
#include "iservmgr_hdi.h"
#include "message_parcel.h"
#include "usb_common.h"
#include "usb_errors.h"
#include "usbd_type.h"

namespace OHOS {
namespace USB {
using OHOS::HDI::ServiceManager::V1_0::IServiceManager;

namespace {
const std::string USBD_SERVICE = "usbd";
}

void UsbdClient::PrintBuffer(const char *title, const uint8_t *buffer, uint32_t length)
{
    std::ostringstream oss;
    if (title == NULL || buffer == nullptr || length == 0) {
        return;
    }
    oss.str("");
    oss << title << " << 二进制数据流[" << length << "字节] >> :";
    for (uint32_t i = 0; i < length; ++i) {
        oss << " " << std::hex << (int)buffer[i];
    }
    oss << "  -->  " << buffer << std::endl;
    USB_HILOGD(MODULE_USB_SERVICE, "%{public}s", oss.str().c_str());
}

sptr<IRemoteObject> UsbdClient::GetUsbdService()
{
    auto serviceManager = IServiceManager::Get();
    if (serviceManager == nullptr) {
        USB_HILOGW(MODULE_USBD, "service manager is nullptr");
        return nullptr;
    }
    auto UsbdService = serviceManager->GetService(USBD_SERVICE.c_str());
    if (UsbdService == nullptr) {
        USB_HILOGW(MODULE_USBD, "Usbd service is nullptr");
        return nullptr;
    }
    return UsbdService;
}

ErrCode UsbdClient::BindUsbdSubscriber(const sptr<UsbdSubscriber> &subscriber)
{
    USB_HILOGD(MODULE_USBD, "BindUsbdSubscriber enter");
    if (subscriber == nullptr) {
        USB_HILOGW(MODULE_USBD, "subscriber is nullptr");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    data.WriteRemoteObject(subscriber);
    return DoDispatch(CMD_BIND_USB_SUBSCRIBER, data, reply);
}

ErrCode UsbdClient::UnbindUsbdSubscriber()
{
    USB_HILOGD(MODULE_USBD, "UnbindUsbdSubscriber enter");
    MessageParcel data;
    MessageParcel reply;
    return DoDispatch(CMD_UNBIND_USB_SUBSCRIBER, data, reply);
}

int32_t UsbdClient::OpenDevice(const UsbDev &dev)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    ErrCode ret = DoDispatch(CMD_FUN_OPEN_DEVICE, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD,
                   "UsbdClient::%{public}s:%{public}d OpenDevice failed ret:%{public}d bus:%{public}d dev:%{public}d",
                   __func__, __LINE__, ret, dev.busNum, dev.devAddr);
    } else {
        USB_HILOGW(MODULE_USBD,
                   "UsbdClient::%{public}s:%{public}d OpenDevice success ret:%{public}d bus:%{public}d dev:%{public}d",
                   __func__, __LINE__, ret, dev.busNum, dev.devAddr);
    }
    return ret;
}

int32_t UsbdClient::GetCurrentFunctions(int32_t &funcs)
{
    USB_HILOGD(MODULE_USBD, "GetCurrentFunctions enter");
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = DoDispatch(CMD_FUN_GET_CURRENT_FUNCTIONS, data, reply);
    if (FAILED(ret)) {
        USB_HILOGW(MODULE_USBD, "CMD_FUN_GET_CURRENT_FUNCTIONS failed, return INVALID_USB_INT_VALUE");
        return ret;
    }
    READ_PARCEL_WITH_RET(reply, Int32, funcs, UEC_SERVICE_READ_PARCEL_ERROR);
    USB_HILOGD(MODULE_USBD, "GetCurrentFunctions funcs %{public}d", funcs);
    return UEC_OK;
}

int32_t UsbdClient::SetCurrentFunctions(int32_t funcs)
{
    USB_HILOGD(MODULE_USBD, "SetCurrentFunctions enter");
    MessageParcel data;
    MessageParcel reply;
    WRITE_PARCEL_WITH_RET(data, Int32, funcs, UEC_SERVICE_WRITE_PARCEL_ERROR);
    USB_HILOGD(MODULE_USBD, "SetCurrentFunctions funcs %{public}d", funcs);
    ErrCode ret = DoDispatch(CMD_FUN_SET_CURRENT_FUNCTIONS, data, reply);
    if (FAILED(ret)) {
        USB_HILOGW(MODULE_USBD, "CMD_FUN_SET_CURRENT_FUNCTIONS failed");
        return ret;
    }
    return UEC_OK;
}

int32_t UsbdClient::SetPortRole(int32_t portId, int32_t powerRole, int32_t dataRole)
{
    USB_HILOGD(MODULE_USBD, "UsbdClient::SetPortRole enter");
    MessageParcel data;
    MessageParcel reply;

    WRITE_PARCEL_WITH_RET(data, Int32, portId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, powerRole, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, dataRole, UEC_SERVICE_WRITE_PARCEL_ERROR);

    ErrCode ret = DoDispatch(CMD_SET_ROLE, data, reply);
    if (FAILED(ret)) {
        USB_HILOGW(MODULE_USBD, "CMD_SET_ROLE failed, return INVALID_STRING_VALUE");
        return ret;
    }
    return UEC_OK;
}

int32_t UsbdClient::QueryPort(int32_t &portId, int32_t &powerRole, int32_t &dataRole, int32_t &mode)
{
    USB_HILOGD(MODULE_USBD, "UsbdClient::queryPort enter");
    MessageParcel data;
    MessageParcel reply;
    ErrCode ret = DoDispatch(CMD_QUERY_PORT, data, reply);
    READ_PARCEL_WITH_RET(reply, Int32, portId, UEC_SERVICE_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(reply, Int32, powerRole, UEC_SERVICE_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(reply, Int32, dataRole, UEC_SERVICE_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(reply, Int32, mode, UEC_SERVICE_READ_PARCEL_ERROR);
    if (FAILED(ret)) {
        USB_HILOGW(MODULE_USBD, "CMD_QUERY_PORT failed, return INVALID_STRING_VALUE");
        return ret;
    }
    USB_HILOGE(MODULE_USBD, "portId:%{public}d powerRole:%{public}d dataRole:%{public}d mode:%{public}d ", portId,
               powerRole, dataRole, mode);
    return UEC_OK;
}

ErrCode UsbdClient::DoDispatch(uint32_t cmd, MessageParcel &data, MessageParcel &reply)
{
    USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d cmd:%{public}d", __func__, __LINE__, cmd);
    auto Usbd = GetUsbdService();
    if (Usbd == nullptr) {
        USB_HILOGE(MODULE_USBD, " get usbd service failed.");
        return UEC_SERVICE_NO_INIT;
    }
    MessageOption option;
    auto ret = Usbd->SendRequest(cmd, data, reply, option);
    USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d SendRequest end cmd:%{public}d ", __func__, __LINE__, cmd);
    if (ret != UEC_OK) {
        USB_HILOGE(MODULE_USBD, "failed to send request, cmd: %{public}d, ret: %{public}d", cmd, ret);
        return ret;
    }
    USB_HILOGD(MODULE_USBD, " success to dispatch cmd: %{public}d", cmd);
    return UEC_OK;
}

int32_t UsbdClient::GetRawDescriptor(const UsbDev &dev, std::vector<uint8_t> &decriptor)
{
    MessageParcel data;
    MessageParcel reply;
    UsbdClient::SetDeviceMessage(data, dev);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_GET_DEVICE_DESCRIPTOR, data, reply);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    ret = UsbdClient::GetBufferMessage(reply, decriptor);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
    } else {
        if (decriptor.size() > 0) {
            PrintBuffer("UsbdClient::GetStringDescriptor", decriptor.data(), decriptor.size());
        }
    }
    return ret;
}

int32_t UsbdClient::GetDeviceDescriptor(const UsbDev &dev, std::vector<uint8_t> &decriptor)
{
    MessageParcel data;
    MessageParcel reply;
    UsbdClient::SetDeviceMessage(data, dev);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_GET_DEVICE_DESCRIPTOR, data, reply);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
        return ret;
    }
    ret = UsbdClient::GetBufferMessage(reply, decriptor);
    return ret;
}

int32_t UsbdClient::GetStringDescriptor(const UsbDev &dev, uint8_t descId, std::vector<uint8_t> &decriptor)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, descId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_GET_STRING_DESCRIPTOR, data, reply);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d strId:%{public}d failed:%{public}d", __func__, __LINE__, descId,
                   ret);
        return ret;
    }
    ret = UsbdClient::GetBufferMessage(reply, decriptor);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d strId:%{public}d failed:%{public}d", __func__, __LINE__, descId,
                   ret);
    }
    if (decriptor.size() > 0) {
        PrintBuffer("UsbdClient::GetStringDescriptor", decriptor.data(), decriptor.size());
    }
    return ret;
}

int32_t UsbdClient::GetConfigDescriptor(const UsbDev &dev, uint8_t descId, std::vector<uint8_t> &decriptor)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, descId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_GET_CONFIG_DESCRIPTOR, data, reply);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d cfgId:%{public}d failed:%{public}d", __func__, __LINE__, descId,
                   ret);
        return ret;
    }
    ret = UsbdClient::GetBufferMessage(reply, decriptor);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d strId:%{public}d failed:%{public}d", __func__, __LINE__, descId,
                   ret);
    }
    if (decriptor.size() > 0) {
        PrintBuffer("UsbdClient::GetStringDescriptor", decriptor.data(), decriptor.size());
    }
    return ret;
}

int32_t UsbdClient::SetConfig(const UsbDev &dev, uint8_t configIndex)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, configIndex, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_SET_CONFIG, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
    }
    return ret;
}

int32_t UsbdClient::GetConfig(const UsbDev &dev, uint8_t &configIndex)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_GET_CONFIG, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
    }
    READ_PARCEL_WITH_RET(data, Uint8, configIndex, UEC_SERVICE_READ_PARCEL_ERROR);
    return ret;
}

int32_t UsbdClient::ClaimInterface(const UsbDev &dev, uint8_t interfaceIndex)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, interfaceIndex, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_CLAIM_INTERFACE, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
    }
    return ret;
}

int32_t UsbdClient::ReleaseInterface(const UsbDev &dev, uint8_t interfaceIndex)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, interfaceIndex, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_RELEASE_INTERFACE, data, reply);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
    }
    return ret;
}

int32_t UsbdClient::SetInterface(const UsbDev &dev, uint8_t interfaceIndex, uint8_t altIndex)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, interfaceIndex, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, altIndex, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_SET_INTERFACE, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
        return ret;
    }
    return UEC_OK;
}

int32_t UsbdClient::BulkTransferRead(const UsbDev &devInfo,
                                     const UsbPipe &pipe,
                                     int32_t timeout,
                                     std::vector<uint8_t> &vdata)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, devInfo);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_SEND_BULK_READ_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    ret = UsbdClient::GetBufferMessage(reply, vdata);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
    }
    return ret;
}

int32_t UsbdClient::BulkTransferWrite(const UsbDev &dev,
                                      const UsbPipe &pipe,
                                      int32_t timeout,
                                      const std::vector<uint8_t> &vdata)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = SetBufferMessage(data, vdata);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    ret = UsbdClient::DoDispatch(CMD_FUN_SEND_BULK_WRITE_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s failed", __func__);
    }
    return ret;
}

int32_t UsbdClient::ControlTransfer(const UsbDev &dev, const UsbCtrlTransfer &ctrl, std::vector<uint8_t> &vdata)
{
    int32_t ret;
    MessageParcel data;
    MessageParcel reply;

    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Int32, ctrl.requestType, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, ctrl.requestCmd, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, ctrl.value, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, ctrl.index, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, ctrl.timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    bool bWrite = (ctrl.requestType & USB_ENDPOINT_DIR_MASK) == USB_ENDPOINT_DIR_OUT;
    if (bWrite) {
        ret = SetBufferMessage(data, vdata);
        if (UEC_OK != ret) {
            USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
            return ret;
        }
    }
    ret = UsbdClient::DoDispatch(CMD_FUN_SEND_CTRL_REQUEST_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    if (!bWrite) {
        ret = GetBufferMessage(reply, vdata);
        if (UEC_OK != ret) {
            USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        } else {
            PrintBuffer("UsbdClient::ControlTransfer", (const uint8_t *)vdata.data(), vdata.size());
        }
    }
    return ret;
}

int32_t UsbdClient::InterruptTransferRead(const UsbDev &dev,
                                          const UsbPipe &pipe,
                                          int32_t timeout,
                                          std::vector<uint8_t> &vdata)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_SEND_INTERRUPT_READ_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
        return ret;
    }

    ret = GetBufferMessage(reply, vdata);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
    } else {
        PrintBuffer("UsbdClient::InterruptTransferRead", (const uint8_t *)vdata.data(), vdata.size());
    }
    return ret;
}

int32_t UsbdClient::InterruptTransferWrite(const UsbDev &dev,
                                           const UsbPipe &pipe,
                                           int32_t timeout,
                                           std::vector<uint8_t> &vdata)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = SetBufferMessage(data, vdata);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    ret = UsbdClient::DoDispatch(CMD_FUN_SEND_INTERRUPT_WRITE_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
    }
    return ret;
}

int32_t UsbdClient::IsoTransferRead(const UsbDev &devInfo,
                                    const UsbPipe &pipe,
                                    int32_t timeout,
                                    std::vector<uint8_t> &vdata)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, devInfo);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_SEND_ISO_READ_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
        return ret;
    }

    ret = GetBufferMessage(reply, vdata);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
    } else {
        PrintBuffer("UsbdClient::IsoTransferRead", (const uint8_t *)vdata.data(), vdata.size());
    }
    return ret;
}

int32_t UsbdClient::IsoTransferWrite(const UsbDev &devInfo,
                                     const UsbPipe &pipe,
                                     int32_t timeout,
                                     std::vector<uint8_t> &vdata)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, devInfo);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = SetBufferMessage(data, vdata);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    ret = UsbdClient::DoDispatch(CMD_FUN_SEND_ISO_WRITE_SYNC, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
    }
    return ret;
}

int32_t UsbdClient::CloseDevice(const UsbDev &dev)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_CLOSE_DEVICE, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d CloseDevice failed ret:%{public}d bus:%{public}d dev:%{public}d",
                   __func__, __LINE__, ret, dev.busNum, dev.devAddr);
    } else {
        USB_HILOGW(MODULE_USBD,
                   "%{public}s:%{public}d CloseDevice success ret:%{public}d bus:%{public}d dev:%{public}d", __func__,
                   __LINE__, ret, dev.busNum, dev.devAddr);
    }
    return ret;
}

int32_t UsbdClient::RequestQueue(const UsbDev &dev,
                                 const UsbPipe &pipe,
                                 const std::vector<uint8_t> &clientData,
                                 const std::vector<uint8_t> &buffer)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);

    int32_t ret = UsbdClient::SetBufferMessage(data, clientData);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }
    ret = UsbdClient::SetBufferMessage(data, buffer);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }

    ret = UsbdClient::DoDispatch(CMD_FUN_REQUEST_QUEUE, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
    }
    return ret;
}

int32_t UsbdClient::RequestWait(const UsbDev &dev,
                                std::vector<uint8_t> &clientData,
                                std::vector<uint8_t> &buffer,
                                int32_t timeout)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Int32, timeout, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_REQUEST_WAIT, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed ret:%{public}d", __func__, __LINE__, ret);
        return ret;
    }

    ret = UsbdClient::GetBufferMessage(reply, clientData);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
        return ret;
    }

    ret = UsbdClient::GetBufferMessage(reply, buffer);
    if (UEC_OK != ret) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
    }

    return ret;
}

int32_t UsbdClient::RequestCancel(const UsbDev &dev, const UsbPipe &pipe)
{
    MessageParcel data;
    MessageParcel reply;
    SetDeviceMessage(data, dev);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.interfaceId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, pipe.endpointId, UEC_SERVICE_WRITE_PARCEL_ERROR);
    int32_t ret = UsbdClient::DoDispatch(CMD_FUN_REQUEST_CANCEL, data, reply);
    if (ret != UEC_OK) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed:%{public}d", __func__, __LINE__, ret);
    }
    return ret;
}

int32_t UsbdClient::SetBufferMessage(MessageParcel &data, const std::vector<uint8_t> &vdata)
{
    uint32_t length = vdata.size();
    const uint8_t *ptr = vdata.data();
    if (!ptr) {
        length = 0;
    }

    if (!data.WriteUint32(length)) {
        USB_HILOGE(MODULE_USBD, "%{public}s:%{public}d failed length:%{public}d", __func__, __LINE__, length);
        return UEC_SERVICE_WRITE_PARCEL_ERROR;
    }
    if ((ptr) && (length > 0) && !data.WriteBuffer(ptr, length)) {
        USB_HILOGE(MODULE_USBD, "%{public}s:%{public}d failed length:%{public}d", __func__, __LINE__, length);
        return UEC_SERVICE_WRITE_PARCEL_ERROR;
    }
    return UEC_OK;
}

int32_t UsbdClient::GetBufferMessage(MessageParcel &data, std::vector<uint8_t> &vdata)
{
    uint32_t dataSize = 0;
    vdata.clear();
    if (!data.ReadUint32(dataSize)) {
        USB_HILOGE(MODULE_USBD, "%{public}s:%{public}d failed", __func__, __LINE__);
        return UEC_SERVICE_READ_PARCEL_ERROR;
    }
    if (dataSize == 0) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d size:%{public}d", __func__, __LINE__, dataSize);
        return UEC_OK;
    }

    const uint8_t *readData = data.ReadUnpadBuffer(dataSize);
    if (readData == nullptr) {
        USB_HILOGW(MODULE_USBD, "%{public}s:%{public}d failed size:%{public}d", __func__, __LINE__, dataSize);
        return UEC_SERVICE_READ_PARCEL_ERROR;
    }
    std::vector<uint8_t> tdata(readData, readData + dataSize);
    vdata.swap(tdata);

    return UEC_OK;
}

int32_t UsbdClient::SetDeviceMessage(MessageParcel &data, const UsbDev &dev)
{
    WRITE_PARCEL_WITH_RET(data, Uint8, dev.busNum, UEC_SERVICE_WRITE_PARCEL_ERROR);
    WRITE_PARCEL_WITH_RET(data, Uint8, dev.devAddr, UEC_SERVICE_WRITE_PARCEL_ERROR);
    return UEC_OK;
}
} // namespace USB
} // namespace OHOS
