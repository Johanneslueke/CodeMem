/* 
 * File:   CSkript.h
 * Author: johannes
 *
 * Created on 20 December 2015, 09:33
 * 
 * 
 * Inspired by http://blog.molecular-matters.com/ and 
 * http://mollyrocket.com/casey/
 * 
 */

#ifndef CSKRIPT_H
#define	CSKRIPT_H

using namespace std;

#include "Application_Memory.h"
#include "Platform.h"





bool LinuxCopyFile(const std::string& SourceSOName, const std::string& TempSOName);
timespec LinuxGetLastWriteTime(const std::string& filename);


template<typename Code>
Code* AppLoadCode(const std::string& source,const std::string& worksource);

template<typename Code>
void AppUnloadCode(Code*);





#endif 	/* CSKRIPT_H */

