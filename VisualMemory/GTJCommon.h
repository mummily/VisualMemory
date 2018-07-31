#pragma once
#include <QString>
#include <Windows.h>

namespace ggp
{
    enum EnumCalcType{GGJ, GCL, GTJ, NONECALCTYPE};
    enum EnumCompareType{THIRTEEN_WITH_SEVENTEEN, SEVENTEEN_WITH_SEVENTEEN, NONECOMPARETYPE};
    enum EnumCompareMeans{ONEBYONE, ALLBYONE};

    enum EnumRunType{CMDRUNTYE, EXERUNTYPE, NONAMERUNTYPE};
    //文件后缀名称
    const QString g_cGTJExtend = "GTJ";
    const QString g_cGCLExtend = "GCL10";
    const QString g_cGGJExtend = "GGJ12";

    //运行程序名称
    const QString g_cGaeaName           = "GAEA.exe";
    const QString g_cCollectName        = "GTJCalcExe.exe";
    const QString g_cCommpareName       = "GTJCompare.exe";
    const QString g_cCommpareCMDName    = "CompareCMD.exe";
    const QString g_cUpgrateName        = "GTJUpgrade.exe";
    const QString g_cUpgrateCMDName     = "GTJUpgradeCMD.exe";
    const QString g_cCheckDataName      = "UnName";

    //配置文件中，运行程序key
    const QString g_cGaeaKey           = "GAEA";
    const QString g_cCollectExeKey     = "GTJCalcExe";
    const QString g_cCommpareKey       = "GTJCompare";
    const QString g_cCommpareCMDKey    = "CompareCMD";
    const QString g_cUpgrateKey        = "GTJUpgrade";
    const QString g_cUpgrateCMDKey     = "GTJUpgradeCMD";
    const QString g_cCheckDataKey      = "UnName";

    //配置文件中，目录key
    const QString g_cCollectMidDataPathKey       = "CollectMidData";        //汇总中间层数据路径
    const QString g_cCollectProjectsPathKey    = "CollectFilePath";      //汇总工程路径
    const QString g_cCprBasePathKey            = "CprBasePath";          //基准工程路径
    const QString g_cCprPathKey                = "CprPath";              //对比工程路径
    const QString g_cCprResultPathKey          = "CprResultPath";        //对比结果路径
    const QString g_cTempCalcPathKey           = "TempCalcPath";         //汇总计算临时目录
    const QString g_cTempProjectFilesPathKey   = "TempProjectFilesPath";

    //结果文件名称
    const QString g_cColResultFileName      = "CollectResult.txt";
    const QString g_cColErrorMessageFileName    = "CollectErrorMessage.txt";
    //exe Path
    struct stConfigureExePath
    {
        QString gaeaPath;           //gaea运行路径
        QString collectExePath;     //独立exe运行路径
        QString comparePath;        //对比工具运行路径
        QString compareCMDPath;     //对比工具运行路径
        QString upgratePath;        //大版本升级运行路径
        QString upgrateCMDPath;     //大版本升级命令行运行路径
        QString checkDataPath;      //中间数据层校验运行路径
    };

    //dir Path
    struct stConfigureDirPath
    {
        QString collectMidDataPath;         //汇总中间层数据路径
        QString collectProjectsPath;        //汇总计算工程路径
        QString cprBasePath;           //基准工程路径
        QString cprPath;               //对比工程路径
        QString cprResultPath;         //对比结果路径
        QString tempCalcPath;          //汇总计算临时目录
        QString tempProjectFilesPath;  
    };

	//exe operation
	namespace ExeOpt
	{
		/*!
		*@brief    获取存在进程的句柄
		*@author caob 2017年8月4日
		*@param[in]    char* strPath	完整路径
		*@return       HANDLE 返回句柄，失败返回nullptr
		*/
        HANDLE getProcessHandle( char* strExePath );

		/*!
		*@brief    启动exe
		*@author caob 2017年8月4日
		*@param[in]    QString strPath	完整路径
		*@param[in]    QString strCmd   命令行参数
		*@return       bool 启动成功返回true
		*/
		bool startExe(QString strPath, QString strCmd);
            
		/*!
		*@brief    停止exe
		*@author caob 2017年8月4日
		*@param[in]    QString strPath  exe名称，不含路径，含扩展名
		*@return       bool 
		*/
		bool stopCalcExe(QString strPath);	

		/*!
		*@brief    exe是否已经启动
		*@author caob 2017年8月4日
		*@param[in]    QString strPath  exe名称，不含路径，含扩展名
		*@return       bool ，存在返回true
		*/
		bool isExist(QString strPath);
	};

	//file operation
	namespace FileOpt
	{
        //获取文件的目录
        QString getFileDir(const QString &filePath);

        //获取文件名称，AIncludeExtensionName:是否包含扩展名
        QString getFileName(const QString &AFilePath, bool AIncludeExtensionName = false);

        //判断两个文件名称是否相同；includeExtendName：是否对比后缀名
        bool sameNameFile(const QString &file1, const QString &file2, bool includeExtendName = false);
         
        //获取文件后缀名
        QString getFileExtend(const QString &filePath);
	};

    namespace DirOpt
    {
        QStringList getFilesByDir( const QString &ADir );
        QStringList getSubFolderFiles(const QString &ADir, const QString filters);
        QStringList getSpecifiedFilesByDir( const QString &ADir , const QString &AFilter);

        int removeFilesByDir( const QString &ADir );
    };
};

