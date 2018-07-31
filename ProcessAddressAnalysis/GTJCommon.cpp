#include "stdafx.h"
#include "GTJCommon.h"
#include <TlHelp32.h>
#include <tchar.h>
#include <QStringList>
#include <qdir.h>
namespace ggp
{
	namespace ExeOpt
	{
		bool stopCalcExe( QString strExePath )
		{
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			PROCESSENTRY32 processInfo;
			processInfo.dwSize = sizeof(PROCESSENTRY32);
			BOOL bFlag = Process32First(hSnap, &processInfo);
			while (bFlag != FALSE)
			{
				if (_tcscmp(processInfo.szExeFile, strExePath.toStdWString().data()) == 0) 
				{
					HANDLE hProc = OpenProcess(PROCESS_TERMINATE, TRUE, processInfo.th32ProcessID);
					TerminateProcess(hProc, 0);
					CloseHandle(hProc);
					return true;
				}  

				bFlag = Process32Next(hSnap, &processInfo);
			}
			CloseHandle(hSnap);

			return false;
		}

        HANDLE getProcessHandle( wchar_t* strExePath )
        {
            HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            PROCESSENTRY32 processInfo;
            processInfo.dwSize = sizeof(PROCESSENTRY32);
            BOOL bFlag = Process32First(hSnap, &processInfo);
            while (bFlag != FALSE)
            {
                if (_tcscmp(processInfo.szExeFile, strExePath) == 0) 
                {
                    CloseHandle(hSnap);
                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.th32ProcessID);
                }  

                bFlag = Process32Next(hSnap, &processInfo);
            }
            CloseHandle(hSnap);
            return nullptr;
        }

		bool startExe( QString strPath, QString strCmd )
		{
			QString strExeFullPara = "\""+ strPath + "\"" + " " + strCmd;
			int nRet = WinExec(strExeFullPara.toLocal8Bit().data(), SW_SHOWNORMAL);
			return (nRet > 32);
		}

		bool isExist(QString strExePath)
		{
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			PROCESSENTRY32 processInfo;
			processInfo.dwSize = sizeof(PROCESSENTRY32);
			BOOL bFlag = Process32First(hSnap, &processInfo);
			while (bFlag != FALSE)
			{
				if (_tcscmp(processInfo.szExeFile, strExePath.toStdWString().data()) == 0) 
				{
					CloseHandle(hSnap);
					return true;
				}  

				bFlag = Process32Next(hSnap, &processInfo);
			}
			CloseHandle(hSnap);
			return false;
		}
	};


    namespace FileOpt
    {
        QString FileOpt::getFileDir( const QString &AFilePath )
        {
            int first = 0;
            for (int i = AFilePath.length(); i!= 0; i--)
            {
                if (AFilePath.at(i - 1) != '/' && AFilePath.at(i - 1) != '\\')  first++;
                else break;
            }
            return AFilePath.left(AFilePath.size() - first);
        }

        QString FileOpt::getFileName( const QString &AFilePath, bool AIncludeExtensionName /*= false*/ )
        {
            int first = 0;
            for (int i = AFilePath.length(); i!= 0; i--)
            {
                if (AFilePath.at(i - 1) != '/' && AFilePath.at(i - 1) != '\\')  first++;
                else break;
            }
            QString filename = AFilePath.right(first);
            if(!AIncludeExtensionName)
                filename = filename.left(filename.indexOf("."));
            return filename;
        }

        bool FileOpt::sameNameFile( const QString &file1, const QString &file2, bool includeExtendName /*= false*/ )
        {
            return (!includeExtendName) ? 
                (getFileName(file1) == getFileName(file2)) ://去掉后缀名对比文件名
            (file1 == file2);    //全字节对比
        }

        QString getFileExtend( const QString &filePath )
        {
            return filePath.right(filePath.length() - filePath.lastIndexOf(".") - 1);
        }

    }
    namespace DirOpt
    {
        QStringList getFilesByDir( const QString &ADir )
        {
            QDir dir(QDir::toNativeSeparators(ADir));
            
            dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
            dir.setSorting(QDir::Size | QDir::Reversed);
            QStringList fileList;
            QFileInfoList list = dir.entryInfoList();
            for (int i = 0; i<list.size(); i++)
            {
                QString file = list.at(i).absoluteFilePath();
                fileList.append(file);
            }
            return fileList;
        }

        int removeFilesByDir( const QString &ADir )
        {
            QStringList files = getFilesByDir(ADir);
            foreach (QString file, files)
            {
                QFile::remove(file);
            }
            //QString cmd = "del /q /f " + QDir::toNativeSeparators(ADir) +"*.*";
            //system(cmd.toStdString().data());
            return false;
        }

        QStringList getSpecifiedFilesByDir( const QString &ADir , const QString &AFilter )
        {
            QDir dir(QDir::toNativeSeparators(ADir));

            dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
            dir.setSorting(QDir::Size | QDir::Reversed);
            QStringList fileList;
            QFileInfoList list = dir.entryInfoList();
            foreach(QFileInfo fileInfo, list)
            {
                if(!fileInfo.isFile()) continue;//不是文件继续，只用于加速，可不加
                //后缀不区分大小写，需要区分直接用“==”
                QString suff = fileInfo.suffix();
                if(0 == suff.compare(AFilter, Qt::CaseInsensitive))
                {
                    fileList.append(fileInfo.absoluteFilePath());
                }
            }
            return fileList;
        }

        QStringList getSubFolderFiles( const QString &ADir, const QString filter )
        {
            //判断路径是否存在
            //QDir dir(ADir);
            //if(!dir.exists())
            //{
            //    return;
            //}

            ////获取所选文件类型过滤器
            //QStringList filters;
            //filters<<filter;

            ////定义迭代器并设置过滤器
            //QDirIterator dir_iterator(ADir,
            //    filters,
            //    QDir::Files | QDir::NoSymLinks,
            //    QDirIterator::Subdirectories);
            QStringList string_list;
            //while(dir_iterator.hasNext())
            //{
            //    dir_iterator.next();
            //    QFileInfo file_info = dir_iterator.fileInfo();
            //    QString absolute_file_path = file_info.absoluteFilePath();
            //    string_list.append(file_path);
            //}
            return string_list;
        }

    }
};
