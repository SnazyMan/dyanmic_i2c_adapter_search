/*
 * main.cpp
 *
 *  Created on: Dec 26, 2016
 *      Author: root
 */

#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <fstream>
#include <fcntl.h>

using namespace std;

int findAdapterNum(string &adapNum, string &nameStr){

	int status = -1;

	//directory holding i2c adapter -- need to search through this
	string i2c_dir = string("/sys/class/i2c-adapter");

	//containers to hold nested directory info
	vector<string> adapter_dirs;
	vector<string> adapter_dirs_inner;

	//open first layer
	DIR *dir = opendir(i2c_dir.c_str());
	if(!dir)
	{
		cout << "findAdapterNum; opendir() failed on:" << i2c_dir << endl;
		return status;
	}
	else
	{
		//store directory info
		struct dirent *ent = NULL;
		errno = 0;

		while(ent = readdir(dir)){
			adapter_dirs.push_back(ent->d_name);
		}
		if(errno){
			cout << "ERROR:" << errno << endl;
			return status;
		}
	}

	for (unsigned int i = 0; i < adapter_dirs.size(); i++)
	{

		//check if directory starts with i2c -- it is an adapter
		if((adapter_dirs[i].compare(0, 3, "i2c")) == 0)
		{
			//build new directory path and open it -- each i2c-x is a directory
			string temp_dir((i2c_dir + "/" + adapter_dirs[i]));
			DIR *dir_inner = opendir(temp_dir.c_str());
			if(!dir)
			{
				cout << "findAdapterNum; opendir() failed on:" << temp_dir << endl;
				return status;
			}
			else
			{
				//get i2c-x directory info
				struct dirent *ent_inner = NULL;
				errno = 0;

				//must clear inner directory container before refilling it w/ new info
				adapter_dirs_inner.clear();
				while(ent_inner = readdir(dir_inner)){
					adapter_dirs_inner.push_back(ent_inner->d_name);
				}
				if(errno){
					cout << "ERROR" << errno << endl;
				}

				for(unsigned int i_inner = 0; i_inner < adapter_dirs_inner.size(); i_inner++)
				{
					//find name file
					if((adapter_dirs_inner[i_inner].compare(0,4,"name")) == 0)
					{
						//open and read file
						string adapterName;
						ifstream nameFile((i2c_dir + "/" + adapter_dirs[i] + "/" +
								adapter_dirs_inner[i_inner]).c_str());
						getline(nameFile, adapterName);
						if((adapterName.compare(nameStr)) == 0)
						{
							//populate string with i2c-x
							adapNum = adapter_dirs[i];
							status = 0;
						}
					}
				}
			}
		}
	}
	return status;
}

int main(){

	string adapNum;
	string desiredAdap("i915 gmbus ssc");
	string devDir("/dev/");
	int status = 0;

	status = findAdapterNum(adapNum, desiredAdap);
	if(status != 0)
	{
		cout << "ERROR: Could not find adapter string";
	}

	//build /dev/i2c-x directory
	devDir += adapNum;

	status = open(devDir.c_str(), O_RDWR);
	if(status != 0)
	{
		cout << "ERROR could not open /dev/i2c-x" << endl;
	}

	//do things to adapter

	return 0;
}

