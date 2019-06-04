/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
 *  
 *  2015 By Richard (qc2105@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Myget - A download accelerator for GNU/Linux
 *  Homepage: http://myget.sf.net
 *  2005 by xiaosuo
 */

#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class HeadDataNode
{
	public:
		HeadDataNode():attrName(NULL), attrValue(NULL), next(NULL){};
		HeadDataNode(const HeadDataNode &that);
		~HeadDataNode();
		HeadDataNode& operator = (const HeadDataNode &that);

		const char *attrName;
		const char *attrValue;
		HeadDataNode *next;
};

class HeadData
{
	public:
		HeadData():head(NULL){};
		HeadData(const HeadDataNode &that);
		~HeadData();
		HeadData& operator = (const HeadData&that);

		// set a attrib property if not exist, add one
		int set_attr(const char *attrName, const char *attrValue);
		// get the attribValue identified by attrName;
		const char* get_attr(const char *attrName);
        // remote all the atts
		void remove_all();

		HeadDataNode *head;
};

#endif // _HEADER_H
