#include "be_cross.h"

int32_t BE_Cross_FileLengthFromHandle(BE_FILE_T fp)
{
	long int origOffset = ftell(fp);
	fseek(fp, 0, SEEK_END);
	long int len = ftell(fp);
	fseek(fp, origOffset, SEEK_SET);
	return len;
}

size_t BE_Cross_readInt8LE(BE_FILE_T fp, void *ptr)
{
	return fread(ptr, 1, 1, fp);
}

size_t BE_Cross_readInt16LE(BE_FILE_T fp, void *ptr)
{
	size_t bytesread = fread(ptr, 1, 2, fp);
#ifdef REFKEEN_ARCH_BIG_ENDIAN
	if (bytesread == 2)
	{
		*(uint16_t *)ptr = BE_Cross_Swap16LE(*(uint16_t *) ptr);
	}
#endif
	return bytesread;
}

size_t BE_Cross_readInt32LE(BE_FILE_T fp, void *ptr)
{
	size_t bytesread = fread(ptr, 1, 4, fp);
#ifdef REFKEEN_ARCH_BIG_ENDIAN
	if (bytesread == 4)
	{
		*(uint32_t *)ptr = BE_Cross_Swap32LE(*(uint32_t *) ptr);
	}
#endif
	return bytesread;
}

size_t BE_Cross_readInt8LEBuffer(BE_FILE_T fp, void *ptr, size_t nbyte)
{
	return fread(ptr, 1, nbyte, fp);
}

size_t BE_Cross_readInt16LEBuffer(BE_FILE_T fp, void *ptr, size_t nbyte)
{
#ifndef REFKEEN_ARCH_BIG_ENDIAN
	return fread(ptr, 1, nbyte, fp);
#else
	size_t result = fread(ptr, 1, nbyte, fp);
	for (uint16_t *currptr = (uint16_t *)ptr, *endptr = currptr + result/2; currptr < endptr; ++currptr)
	{
		*currptr = BE_Cross_Swap16LE(*currptr);
	}
	return result;
#endif
}

size_t BE_Cross_readInt32LEBuffer(BE_FILE_T fp, void *ptr, size_t nbyte)
{
#ifndef REFKEEN_ARCH_BIG_ENDIAN
	return fread(ptr, 1, nbyte, fp);
#else
	size_t result = fread(ptr, 1, nbyte, fp);
	for (uint32_t *currptr = (uint32_t *)ptr, *endptr = currptr + result/4; currptr < endptr; ++currptr)
	{
		*currptr = BE_Cross_Swap32LE(*currptr);
	}
	return result;
#endif
}

// This exists for the EGAHEADs from the Catacombs
size_t BE_Cross_readInt24LEBuffer(BE_FILE_T fp, void *ptr, size_t nbyte)
{
#ifndef REFKEEN_ARCH_BIG_ENDIAN
	return fread(ptr, 1, nbyte, fp);
#else
	size_t result = fread(ptr, 1, nbyte, fp);
	uint8_t tempbyte;
	// Let's ensure there's no buffer overflow in case nbyte is not divisible by 3
	for (uint8_t *currbyteptr = (uint8_t *)ptr, *endbyteptr = currbyteptr + (nbyte/3)*3; currbyteptr < endbyteptr; currbyteptr += 3)
	{
		tempbyte = *currbyteptr;
		*currbyteptr = *(currbyteptr+2);
		*(currbyteptr+2) = tempbyte;
	}
	return result;
#endif
}

size_t BE_Cross_writeInt8LE(BE_FILE_T fp, const void *ptr)
{
	return fwrite(ptr, 1, 1, fp);
}

size_t BE_Cross_writeInt16LE(BE_FILE_T fp, const void *ptr)
{
#ifndef REFKEEN_ARCH_BIG_ENDIAN
	return fwrite(ptr, 1, 2, fp);
#else
	uint16_t val = BE_Cross_Swap16LE(*(uint16_t *) ptr);
	return fwrite(&val, 1, 2, fp);
#endif
}

size_t BE_Cross_writeInt32LE(BE_FILE_T fp, const void *ptr)
{
#ifndef REFKEEN_ARCH_BIG_ENDIAN
	return fwrite(ptr, 1, 4, fp);
#else
	uint32_t val = BE_Cross_Swap32LE(*(uint32_t *) ptr);
	return fwrite(&val, 1, 4, fp);
#endif
}

size_t BE_Cross_writeInt8LEBuffer(BE_FILE_T fp, const void *ptr, size_t nbyte)
{
	return fwrite(ptr, 1, nbyte, fp);
}

size_t BE_Cross_writeInt16LEBuffer(BE_FILE_T fp, const void *ptr, size_t nbyte)
{
#ifndef REFKEEN_ARCH_BIG_ENDIAN
	return fwrite(ptr, 1, nbyte, fp);
#else
	size_t result = 0;
	for (uint16_t *currptr = (uint16_t *)ptr, *endptr = currptr + nbyte/2; currptr < endptr; ++currptr)
	{
		uint16_t val = BE_Cross_Swap16LE(*currptr);
		size_t bytesread = fwrite(&val, 1, 2, fp);
		result += bytesread;
		if (bytesread < 2)
		{
			break;
		}
	}
	return result;
#endif
}

size_t BE_Cross_read_boolean_From16LE(BE_FILE_T fp, bool *ptr)
{
	uint16_t val;
	size_t bytesread = fread(&val, 1, 2, fp);
	if (bytesread == 2)
	{
		*ptr = val; // No need to swap byte-order here
	}
	return bytesread;
}

size_t BE_Cross_read_boolean_From32LE(BE_FILE_T fp, bool *ptr)
{
	uint32_t val;
	size_t bytesread = fread(&val, 1, 4, fp);
	if (bytesread == 4)
	{
		*ptr = val; // No need to swap byte-order here
	}
	return bytesread;
}

size_t BE_Cross_read_booleans_From16LEBuffer(BE_FILE_T fp, bool *ptr, size_t nbyte)
{
	uint16_t val;
	size_t totalbytesread = 0, currbytesread;
	for (size_t curbyte = 0; curbyte < nbyte; curbyte += 2, ++ptr)
	{
		currbytesread = fread(&val, 1, 2, fp);
		totalbytesread += currbytesread;
		if (currbytesread < 2)
		{
			return totalbytesread;
		}
		*ptr = val; // No need to swap byte-order here
	}
	return totalbytesread;
}

size_t BE_Cross_read_booleans_From32LEBuffer(BE_FILE_T fp, bool *ptr, size_t nbyte)
{
	uint32_t val;
	size_t totalbytesread = 0, currbytesread;
	for (size_t curbyte = 0; curbyte < nbyte; curbyte += 4, ++ptr)
	{
		currbytesread = fread(&val, 1, 4, fp);
		totalbytesread += currbytesread;
		if (currbytesread < 4)
		{
			return totalbytesread;
		}
		*ptr = val; // No need to swap byte-order here
	}
	return totalbytesread;
}


size_t BE_Cross_write_boolean_To16LE(BE_FILE_T fp, const bool *ptr)
{
	uint16_t val = BE_Cross_Swap16LE((uint16_t)(*ptr)); // Better to swap just in case...
	return fwrite(&val, 1, 2, fp);
}

size_t BE_Cross_write_boolean_To32LE(BE_FILE_T fp, const bool *ptr)
{
	uint32_t val = BE_Cross_Swap32LE((uint32_t)(*ptr)); // Better to swap just in case...
	return fwrite(&val, 1, 4, fp);
}

size_t BE_Cross_write_booleans_To16LEBuffer(BE_FILE_T fp, const bool *ptr, size_t nbyte)
{
	uint16_t val;
	size_t totalbyteswritten = 0, currbyteswritten;
	for (size_t curbyte = 0; curbyte < nbyte; curbyte += 2, ++ptr)
	{
		val = BE_Cross_Swap16LE((uint16_t)(*ptr)); // Better to swap just in case...
		currbyteswritten = fwrite(&val, 1, 2, fp);
		totalbyteswritten += currbyteswritten;
		if (currbyteswritten < 2)
		{
			return totalbyteswritten;
		}
	}
	return totalbyteswritten;
}

size_t BE_Cross_write_booleans_To32LEBuffer(BE_FILE_T fp, const bool *ptr, size_t nbyte)
{
	uint32_t val;
	size_t totalbyteswritten = 0, currbyteswritten;
	for (size_t curbyte = 0; curbyte < nbyte; curbyte += 4, ++ptr)
	{
		val = BE_Cross_Swap16LE((uint32_t)(*ptr)); // Better to swap just in case...
		currbyteswritten = fwrite(&val, 1, 4, fp);
		totalbyteswritten += currbyteswritten;
		if (currbyteswritten < 4)
		{
			return totalbyteswritten;
		}
	}
	return totalbyteswritten;
}
