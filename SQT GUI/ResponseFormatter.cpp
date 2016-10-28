#include "StdAfx.h"
#include "ResponseFormatter.h"
#include <Windows.h>

void InitResultBuffer( DataStore *Buf )
{
    if( Buf == NULL )
        return;
    Buf->Count = 0;
    Buf->Size = sizeof( DataStore );
    Buf->Ver = BUFFSTORE_CURRENT_VERSION;
    Buf->NoRealloc = 0;
}

DataStore *DS_PackToResult( DataStore *Buf, int Type, int Len, const void *val, ValStore **StoredAt, int *Error )
{
    //protect it from early exit bad values
    if( StoredAt != NULL )
        *StoredAt = NULL;
    if( Error != NULL )
        *Error = DS_ERROR_NONE;

    // make sure we have enough space to store new data
    int NeedSize = sizeof( ValStore ) + Len;
    if( Buf != NULL )
        NeedSize += Buf->Size;
    if( Buf == NULL || Buf->NoRealloc == 0 )
    {
        int FirstTimeInit = 0;
        if( Buf == NULL )
        {
            NeedSize += sizeof( DataStore );
            FirstTimeInit = 1;
        }
    
        // readjust buffer so that we can fit in it
        if( Buf == NULL || NeedSize > Buf->AllocSize )
        {
            Buf = (DataStore *)realloc( Buf, NeedSize );
            Buf->AllocSize = NeedSize; // if we extended our buffer than update our max size also
        }

        // First time init
        if( FirstTimeInit == 1 )
            InitResultBuffer( Buf );

    }
    // we can not reallocate external allocated buffers
    else 
    {
        if( Buf->AllocSize <= Buf->Size + sizeof( ValStore ) + Len )
        {
            //we should somehow signal that we were not able to store this value
            if( Error != NULL )
                *Error = DS_ERROR_COULD_NOT_STORE_VALUE;
            return Buf;
        }
     }

    // store the value
    ValStore *Header = (ValStore *)((char*)Buf + Buf->Size);
    Header->Type = Type;
    Header->Len = Len;

    //write the value
    if( val != NULL )
        memcpy( &Header->Value, val, Len );
    else
        memset( &Header->Value, 0, Len );

    // update header that we added a new value
    Buf->Count++;
    Buf->Size = NeedSize;

    // sometimes we want the value store location to be given back to the caller
    if( StoredAt != NULL )
        *StoredAt = Header;

    return Buf;
}

DataStore *DS_PackToResult( DataStore *Buf, int Type, const char val )
{
    return DS_PackToResult( Buf, Type, 1, &val );
}
DataStore *DS_PackToResult( DataStore *Buf, int Type, const short val )
{
    return DS_PackToResult( Buf, Type, 2, &val );
}
DataStore *DS_PackToResult( DataStore *Buf, int Type, const int val )
{
    return DS_PackToResult( Buf, Type, 4, &val );
}

//this is just a Demo implementation hot to recursively pack packets. Actuall packer initialization and usage is missing
DataStore *DS_CompressData( DataStore *Buf )
{
    //this is a dummy Packer, You would replace this function with a real ZIP deflate
    #define ZIP_DEFLATE( InputBuf, InputSize, OutputBuff, OutputSize ) OutputBuff = (char*)InputBuf; OutputSize = InputSize;
    //compress the whole packet into a new value
    int CompressedLength;
    char *CompressedBuf;
    ZIP_DEFLATE( Buf, Buf->Size, CompressedBuf, CompressedLength );
    return DS_PackToResult( NULL, VT_ZIP_COMPRESSED_PACKET, CompressedLength, CompressedBuf );
}

ValStore *DS_GetValue( DataStore *Buf, int index )
{
    //sanity checks
    if( Buf == NULL )
        return NULL;
    if( index >= Buf->Count )
        return NULL;

    //seek to index
    char *Ret = (char*)Buf + sizeof( DataStore );
    for( int i = 0; i != index && Buf->Size > (int)Ret - (int)Buf; i++ )
        Ret += ((ValStore*)Ret)->Len + sizeof( ValStore );

    //return value
    return (ValStore *)Ret;
}

void Dispose_DS( DataStore **Buf )
{
    if( Buf == NULL )
        return;
    if( *Buf != NULL )
        free( *Buf );
    *Buf = NULL;
}