/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
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

#include <cstring>
#include <cassert>

#include <QDebug>

#include "header.h"
#include "utils.h"

/*********************************************
 * class HeadDataNode implement
 * *******************************************/
//HeadDataNode::HeadDataNode(const HeadDataNode &that);
HeadDataNode::~HeadDataNode()
{
    //delete[] attrName;
    //delete[] attrValue;
};

//HeadDataNode& 
//HeadDataNode::operator = (const HeadDataNode &that);

/*******************************************
 * class HeadData implement 
 *******************************************/
//HeadData::HeadData(const HeadDataNode &that);
HeadData::~HeadData()
{
    remove_all();
};

//HeadData&
//HeadData::operator = (const HeadData&that);

const char* 
HeadData::get_attr(const char *attrName)
{
    HeadDataNode *it;

    assert(attrName != NULL);

    for(it = head; it != NULL; it = it->next){
        if(strcmp(attrName, it->attrName) == 0)
            return it->attrValue;
    }

	return NULL;
}

int 
HeadData::set_attr(const char *attrName, const char *attrValue)
{
    HeadDataNode *it;

    assert(attrName != NULL && attrValue != NULL);

    for(it = head; it != NULL; it = it->next){
        if(strcmp(attrName, it->attrName) == 0)
            break;
    }

    if(it != NULL){
        delete[] it->attrValue;
        it->attrValue = StrDup(attrValue);
        return 0;
    }else{
        it = new HeadDataNode;
        it->attrName = StrDup(attrName);
        it->attrValue = StrDup(attrValue);
        it->next = head;
        head = it;
        return 1;
    }
}

void
HeadData::remove_all()
{
    HeadDataNode *it;

    for(it = head; it != NULL; it = it->next){
        delete it;
    }

    head = NULL;
}


