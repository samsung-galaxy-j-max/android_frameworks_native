/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/NativeHandle.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>
#include <utils/Vector.h>

#include <binder/Parcel.h>
#include <binder/IInterface.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/IProducerListener.h>

namespace android {
// ----------------------------------------------------------------------------

enum {
    REQUEST_BUFFER = IBinder::FIRST_CALL_TRANSACTION,
    SET_BUFFER_COUNT,
    DEQUEUE_BUFFER,
    DETACH_BUFFER,
    DETACH_NEXT_BUFFER,
    ATTACH_BUFFER,
    QUEUE_BUFFER,
    CANCEL_BUFFER,
    QUERY,
    CONNECT,
    DISCONNECT,
    SET_SIDEBAND_STREAM,
    ALLOCATE_BUFFERS,
};

class BpGraphicBufferProducer : public BpInterface<IGraphicBufferProducer>
{
public:
    BpGraphicBufferProducer(const sp<IBinder>& impl)
        : BpInterface<IGraphicBufferProducer>(impl)
    {
    }

    virtual status_t requestBuffer(int bufferIdx, sp<GraphicBuffer>* buf) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(bufferIdx);
        status_t result =remote()->transact(REQUEST_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        bool nonNull = reply.readInt32();
        if (nonNull) {
            *buf = new GraphicBuffer();
            result = reply.read(**buf);
            if(result != NO_ERROR) {
                (*buf).clear();
                return result;
            }
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t setBufferCount(int bufferCount)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(bufferCount);
        status_t result =remote()->transact(SET_BUFFER_COUNT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t dequeueBuffer(int *buf, sp<Fence>* fence, bool async,
            uint32_t w, uint32_t h, uint32_t format, uint32_t usage) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(async);
        data.writeInt32(w);
        data.writeInt32(h);
        data.writeInt32(format);
        data.writeInt32(usage);
        status_t result = remote()->transact(DEQUEUE_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        *buf = reply.readInt32();
        bool nonNull = reply.readInt32();
        if (nonNull) {
            *fence = new Fence();
            reply.read(**fence);
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t detachBuffer(int slot) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(slot);
        status_t result = remote()->transact(DETACH_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t detachNextBuffer(sp<GraphicBuffer>* outBuffer,
            sp<Fence>* outFence) {
        if (outBuffer == NULL) {
            ALOGE("detachNextBuffer: outBuffer must not be NULL");
            return BAD_VALUE;
        } else if (outFence == NULL) {
            ALOGE("detachNextBuffer: outFence must not be NULL");
            return BAD_VALUE;
        }
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        status_t result = remote()->transact(DETACH_NEXT_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        if (result == NO_ERROR) {
            bool nonNull = reply.readInt32();
            if (nonNull) {
                *outBuffer = new GraphicBuffer;
                reply.read(**outBuffer);
            }
            nonNull = reply.readInt32();
            if (nonNull) {
                *outFence = new Fence;
                reply.read(**outFence);
            }
        }
        return result;
    }

    virtual status_t attachBuffer(int* slot, const sp<GraphicBuffer>& buffer) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.write(*buffer.get());
        status_t result = remote()->transact(ATTACH_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        *slot = reply.readInt32();
        result = reply.readInt32();
        return result;
    }

    virtual status_t queueBuffer(int buf,
            const QueueBufferInput& input, QueueBufferOutput* output) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(buf);
        data.write(input);
        status_t result = remote()->transact(QUEUE_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
        result = reply.readInt32();
        return result;
    }

    virtual void cancelBuffer(int buf, const sp<Fence>& fence) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(buf);
        data.write(*fence.get());
        remote()->transact(CANCEL_BUFFER, data, &reply);
    }

    virtual int query(int what, int* value) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(what);
        status_t result = remote()->transact(QUERY, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        value[0] = reply.readInt32();
        result = reply.readInt32();
        return result;
    }

    virtual status_t connect(const sp<IProducerListener>& listener,
            int api, bool producerControlledByApp, QueueBufferOutput* output) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        if (listener != NULL) {
            data.writeInt32(1);
            data.writeStrongBinder(listener->asBinder());
        } else {
            data.writeInt32(0);
        }
        data.writeInt32(api);
        data.writeInt32(producerControlledByApp);
        status_t result = remote()->transact(CONNECT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
        result = reply.readInt32();
        return result;
    }

    virtual status_t disconnect(int api) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(api);
        status_t result =remote()->transact(DISCONNECT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t setSidebandStream(const sp<NativeHandle>& stream) {
        Parcel data, reply;
        status_t result;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        if (stream.get()) {
            data.writeInt32(true);
            data.writeNativeHandle(stream->handle());
        } else {
            data.writeInt32(false);
        }
        if ((result = remote()->transact(SET_SIDEBAND_STREAM, data, &reply)) == NO_ERROR) {
            result = reply.readInt32();
        }
        return result;
    }

    virtual void allocateBuffers(bool async, uint32_t width, uint32_t height,
            uint32_t format, uint32_t usage) {
        Parcel data, reply;
        data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(async));
        data.writeInt32(static_cast<int32_t>(width));
        data.writeInt32(static_cast<int32_t>(height));
        data.writeInt32(static_cast<int32_t>(format));
        data.writeInt32(static_cast<int32_t>(usage));
        status_t result = remote()->transact(ALLOCATE_BUFFERS, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("allocateBuffers failed to transact: %d", result);
        }
    }
};

IMPLEMENT_META_INTERFACE(GraphicBufferProducer, "android.gui.IGraphicBufferProducer");

// ----------------------------------------------------------------------

status_t BnGraphicBufferProducer::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case REQUEST_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int bufferIdx   = data.readInt32();
            sp<GraphicBuffer> buffer;
            int result = requestBuffer(bufferIdx, &buffer);
            reply->writeInt32(buffer != 0);
            if (buffer != 0) {
                reply->write(*buffer);
            }
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case SET_BUFFER_COUNT: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int bufferCount = data.readInt32();
            int result = setBufferCount(bufferCount);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case DEQUEUE_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            bool async      = data.readInt32();
            uint32_t w      = data.readInt32();
            uint32_t h      = data.readInt32();
            uint32_t format = data.readInt32();
            uint32_t usage  = data.readInt32();
            int buf = 0;
            sp<Fence> fence;
            int result = dequeueBuffer(&buf, &fence, async, w, h, format, usage);
            reply->writeInt32(buf);
            reply->writeInt32(fence != NULL);
            if (fence != NULL) {
                reply->write(*fence);
            }
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case DETACH_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int slot = data.readInt32();
            int result = detachBuffer(slot);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case DETACH_NEXT_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            sp<GraphicBuffer> buffer;
            sp<Fence> fence;
            int32_t result = detachNextBuffer(&buffer, &fence);
            reply->writeInt32(result);
            if (result == NO_ERROR) {
                reply->writeInt32(buffer != NULL);
                if (buffer != NULL) {
                    reply->write(*buffer);
                }
                reply->writeInt32(fence != NULL);
                if (fence != NULL) {
                    reply->write(*fence);
                }
            }
            return NO_ERROR;
        } break;
        case ATTACH_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            sp<GraphicBuffer> buffer = new GraphicBuffer();
            data.read(*buffer.get());
            int slot = 0;
            int result = attachBuffer(&slot, buffer);
            reply->writeInt32(slot);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case QUEUE_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int buf = data.readInt32();
            QueueBufferInput input(data);
            QueueBufferOutput* const output =
                    reinterpret_cast<QueueBufferOutput *>(
                            reply->writeInplace(sizeof(QueueBufferOutput)));
            memset(output, 0, sizeof(QueueBufferOutput));
            status_t result = queueBuffer(buf, input, output);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case CANCEL_BUFFER: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int buf = data.readInt32();
            sp<Fence> fence = new Fence();
            data.read(*fence.get());
            cancelBuffer(buf, fence);
            return NO_ERROR;
        } break;
        case QUERY: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int value = 0;
            int what = data.readInt32();
            int res = query(what, &value);
            reply->writeInt32(value);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case CONNECT: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            sp<IProducerListener> listener;
            if (data.readInt32() == 1) {
                listener = IProducerListener::asInterface(data.readStrongBinder());
            }
            int api = data.readInt32();
            bool producerControlledByApp = data.readInt32();
            QueueBufferOutput* const output =
                    reinterpret_cast<QueueBufferOutput *>(
                            reply->writeInplace(sizeof(QueueBufferOutput)));
            memset(output, 0, sizeof(QueueBufferOutput));
            status_t res = connect(listener, api, producerControlledByApp, output);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case DISCONNECT: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int api = data.readInt32();
            status_t res = disconnect(api);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case SET_SIDEBAND_STREAM: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            sp<NativeHandle> stream;
            if (data.readInt32()) {
                stream = NativeHandle::create(data.readNativeHandle(), true);
            }
            status_t result = setSidebandStream(stream);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case ALLOCATE_BUFFERS:
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            bool async = static_cast<bool>(data.readInt32());
            uint32_t width = static_cast<uint32_t>(data.readInt32());
            uint32_t height = static_cast<uint32_t>(data.readInt32());
            uint32_t format = static_cast<uint32_t>(data.readInt32());
            uint32_t usage = static_cast<uint32_t>(data.readInt32());
            allocateBuffers(async, width, height, format, usage);
            return NO_ERROR;
    }
    return BBinder::onTransact(code, data, reply, flags);
}

// ----------------------------------------------------------------------------

IGraphicBufferProducer::QueueBufferInput::QueueBufferInput(const Parcel& parcel) {
    parcel.read(*this);
}

size_t IGraphicBufferProducer::QueueBufferInput::getFlattenedSize() const {
    return sizeof(timestamp)
         + sizeof(isAutoTimestamp)
         + sizeof(crop)
#ifdef QCOM_HARDWARE
         + sizeof(dirtyRect)
#endif /* QCOM_HARDWARE */
         + sizeof(scalingMode)
         + sizeof(transform)
         + sizeof(stickyTransform)
         + sizeof(async)
         + fence->getFlattenedSize();
}

size_t IGraphicBufferProducer::QueueBufferInput::getFdCount() const {
    return fence->getFdCount();
}

status_t IGraphicBufferProducer::QueueBufferInput::flatten(
        void*& buffer, size_t& size, int*& fds, size_t& count) const
{
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }
    FlattenableUtils::write(buffer, size, timestamp);
    FlattenableUtils::write(buffer, size, isAutoTimestamp);
    FlattenableUtils::write(buffer, size, crop);
#ifdef QCOM_HARDWARE
    FlattenableUtils::write(buffer, size, dirtyRect);
#endif /* QCOM_HARDWARE */
    FlattenableUtils::write(buffer, size, scalingMode);
    FlattenableUtils::write(buffer, size, transform);
    FlattenableUtils::write(buffer, size, stickyTransform);
    FlattenableUtils::write(buffer, size, async);
    return fence->flatten(buffer, size, fds, count);
}

status_t IGraphicBufferProducer::QueueBufferInput::unflatten(
        void const*& buffer, size_t& size, int const*& fds, size_t& count)
{
    size_t minNeeded =
              sizeof(timestamp)
            + sizeof(isAutoTimestamp)
            + sizeof(crop)
#ifdef QCOM_HARDWARE
            + sizeof(dirtyRect)
#endif /* QCOM_HARDWARE */
            + sizeof(scalingMode)
            + sizeof(transform)
            + sizeof(stickyTransform)
            + sizeof(async);

    if (size < minNeeded) {
        return NO_MEMORY;
    }

    FlattenableUtils::read(buffer, size, timestamp);
    FlattenableUtils::read(buffer, size, isAutoTimestamp);
    FlattenableUtils::read(buffer, size, crop);
#ifdef QCOM_HARDWARE
    FlattenableUtils::read(buffer, size, dirtyRect);
#endif /* QCOM_HARDWARE */
    FlattenableUtils::read(buffer, size, scalingMode);
    FlattenableUtils::read(buffer, size, transform);
    FlattenableUtils::read(buffer, size, stickyTransform);
    FlattenableUtils::read(buffer, size, async);

    fence = new Fence();
    return fence->unflatten(buffer, size, fds, count);
}

}; // namespace android
