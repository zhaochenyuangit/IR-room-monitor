/*******************************************************************************
 Copyright (C) <2015>, <Panasonic Corporation>
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1.	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	The name of copyright holders may not be used to endorse or promote products derived from this software without specific prior written permission.
4.	This software code may only be redistributed and used in connection with a grid-eye product.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR POFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND OR ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the authors and should not be interpreted as representing official policies, either expressed or implied, of the FreeBSD Project. 

 ******************************************************************************/
#include "detect.h"
#include "grideye_api_common.h"
UCHAR ucAMG_PUB_ODT_CalcDataLabeling8(UCHAR ucWidth, UCHAR ucHeight, UCHAR ucMark, USHORT usArea, UCHAR *pucImg, USHORT *pusSearchList)
{
	USHORT usImg = 0;
	USHORT usSize = ucWidth * ucHeight;
	UCHAR ucDetectNum = 0;

	for (usImg = 0; usImg < usSize; usImg++)
	{
		UCHAR ucLabelNum = 0;
		USHORT usIndex = 0;
		USHORT usIndexAdd = 0;

		if (ucMark == pucImg[usImg])
		{
			pucImg[usImg] = 0;
			pusSearchList[usIndex] = usImg;
			usIndexAdd = 1;
		}

		while (usIndex < usIndexAdd)
		{
			UCHAR ucX = pusSearchList[usIndex] % ucWidth;
			UCHAR ucY = pusSearchList[usIndex] / ucWidth;
			{
				if (0 <= (ucY - 1)) // 不是第一行
				{
					USHORT usCheckIndex = (ucX) + (ucY - 1) * ucWidth;
					if (ucMark == pucImg[usCheckIndex]) // 看上面一个像素是不是物体
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (ucHeight > (ucY + 1)) //不是最后一行
				{
					USHORT usCheckIndex = (ucX) + (ucY + 1) * ucWidth;
					if (ucMark == pucImg[usCheckIndex]) // 看下面一个像素是不是物体
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
			}
			if (0 <= (ucX - 1)) //不是最左
			{
				{
					USHORT usCheckIndex = (ucX - 1) + (ucY)*ucWidth; //正左边
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (0 <= (ucY - 1))
				{
					USHORT usCheckIndex = (ucX - 1) + (ucY - 1) * ucWidth; //左上
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (ucHeight > (ucY + 1)) //左下
				{
					USHORT usCheckIndex = (ucX - 1) + (ucY + 1) * ucWidth;
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
			}
			if (ucWidth > (ucX + 1)) //不是最右
			{
				{
					USHORT usCheckIndex = (ucX + 1) + (ucY)*ucWidth; //正右边
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (0 <= (ucY - 1))
				{
					USHORT usCheckIndex = (ucX + 1) + (ucY - 1) * ucWidth; //右上
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
				if (ucHeight > (ucY + 1))
				{
					USHORT usCheckIndex = (ucX + 1) + (ucY + 1) * ucWidth; //右下
					if (ucMark == pucImg[usCheckIndex])
					{
						pucImg[usCheckIndex] = 0;
						pusSearchList[usIndexAdd++] = usCheckIndex;
					}
				}
			}
			usIndex++;
		}
		
		if (usIndex <= usArea) //小于三个扩容后像素则认为是没有检测到物体
		{
			ucLabelNum = 0;
		}
		else
		{
			ucDetectNum++;
			ucLabelNum = ucDetectNum;
		}

		{
			USHORT usImg2 = 0;
			for (usImg2 = 0; usImg2 < usIndex; usImg2++)
			{
				pucImg[pusSearchList[usImg2]] = ucLabelNum;
			}
		}
	}

	return (ucDetectNum);
}