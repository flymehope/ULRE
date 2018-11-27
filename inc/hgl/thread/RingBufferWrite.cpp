#ifndef HGL_THREAD_RING_BUFFER_WRITE_SOURCE
#define HGL_THREAD_RING_BUFFER_WRITE_SOURCE
#include<string.h>

namespace hgl
{
    /**
    * 安全取得可写入数据长度
    */
    template<typename T>
    int RingBuffer<T>::SafeGetWriteSize()
    {
        Lock();

        const int result=_GetWriteSize();

        Unlock();

        return(result);
    }

    template<typename T>
    int RingBuffer<T>::WriteStart()
    {
        ClampPosition();

        read_off=read_pos%buffer_size;

        write_max=_GetWriteSize();

        write_count=0;
        write_cur=write_pos;

        return write_max;
    }

    template<typename T>
    int RingBuffer<T>::_SafeWriteStart()
    {
        WriteStart();

        if(write_max<=0)
            Unlock();

        return write_max;
    }

    /**
    * 尝试开始写入
    * @return >0 成功开始，可写入的数据长度
    * @return =0 没有可以写入的空间
    * @return <0 暂时不可写入
    */
    template<typename T>
    int RingBuffer<T>::SafeTryWriteStart()
    {
        if(!TryLock())
            return(-1);

        return _SafeWriteStart();
    }

    /**
    * 开始写入数据,如果没有空间会立即关闭缓冲区，不必再次调用SafeWriteEnd
    * @return 可写入的数据长度
    */
    template<typename T>
    int RingBuffer<T>::SafeWriteStart()
    {
        Lock();

        return _SafeWriteStart();
    }

    /**
    * 结束写入数据
    * @param data 要写入的数据
    * @param size 要写入的数据长度
    * @return 实际写入的数据长度
    */
    template<typename T>
    int RingBuffer<T>::Write(const T *data,int size)
    {
        if(!data||size<=0)return(-1);

        const int result=_Write(data,size);

        write_count+=result;

        return result;
    }

    /**
    * 写入结束
    * @return 返回写入的数据长度
    */
    template<typename T>
    int RingBuffer<T>::WriteEnd()
    {
        const int result=write_count;

        if(result)
            write_pos=write_cur;

        return result;
    }

    /**
    * 安全写入结束
    * @return 返回写入的数据长度
    */
    template<typename T>
    int RingBuffer<T>::SafeWriteEnd()
    {
        const int result=WriteEnd();

        Unlock();

        return result;
    }

    /**
    * 安全写入数据,此函数会直接开锁解锁，用于少量的一次性处理。如大量的数据要分次写入，请使用SafeWriteStart/SafeWriteEnd
    * @param data 要写入的数据
    * @param size 要写入的数据长度
    * @return 实际写入的数据长度
    * @return -1 出错
    */
    template<typename T>
    int RingBuffer<T>::SafeWrite(const T *data,int size)
    {
        if(!data||size<=0)return(-1);

        if(SafeWriteStart()<=0)
            return(-1);

        Write(data,size);

        return SafeWriteEnd();
    }

    template<typename T>
    int RingBuffer<T>::_Write(const T *data,int size)
    {
        if(size<=0||write_max<=0)return(0);

        if(size>write_max)
            size=write_max;

        if(size>0)
        {
            const int temp_write=write_cur%buffer_size;

            if(read_off<=temp_write)
            {
                int temp=buffer_size-temp_write;

                if(size>temp)
                {
                    memcpy(buffer+temp_write,data,temp*sizeof(T));

                    memcpy(buffer,(char *)(data+temp),(size-temp)*sizeof(T));
                }
                else
                {
                    memcpy(buffer+temp_write,data,size*sizeof(T));
                }
            }
            else
            {
                memcpy(buffer+temp_write,data,size*sizeof(T));
            }

            write_cur+=size;
            write_max-=size;
        }

        return size;
    }
}//namespace hgl
#endif//HGL_THREAD_RING_BUFFER_WRITE_SOURCE
