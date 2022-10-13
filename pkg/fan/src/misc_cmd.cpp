// **************************************************************************
// File       [ misc_cmd.cpp ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/09/28 created ]
// **************************************************************************

#include "common/tm_usage.h"

#include "misc_cmd.h"

using namespace std;
using namespace CommonNs;
using namespace FanNs;

//{{{ ReportPatFormatCmd::ReportPatFormatCmd()
ReportPatFormatCmd::ReportPatFormatCmd(const std::string &name) : Cmd(name)
{
	optMgr_.setName(name);
	optMgr_.setShortDes("report pattern format");
	optMgr_.setDes("report pattern format");
	Opt *opt = new Opt(Opt::BOOL, "print usage", "");
	opt->addFlag("h");
	opt->addFlag("help");
	optMgr_.regOpt(opt);
}
ReportPatFormatCmd::~ReportPatFormatCmd() {}
//}}}
//{{{ bool ReportPatFormatCmd::exec()
bool ReportPatFormatCmd::exec(const vector<string> &argv)
{
	optMgr_.parse(argv);

	if (optMgr_.isFlagSet("h"))
	{
		optMgr_.usage();
		return true;
	}

	cout << "<PI names> | <PPI names> | <PO names>" << endl;
	cout << "<BASIC_SCAN | LAUNCH_CAPTURE | LAUNCH_SHIFT>" << endl;
	cout << "_num_of_pattern_<number>" << endl;
	cout << "[ _pattern_<number> <PI1> | [PI2] | [PPI] | ";
	cout << "[SI] | [PO1] | [PO2] | [PPO] ]..." << endl;
	return true;
} //}}}

//{{{ ReportMemUsgCmd::ReportMemUsgCmd()
ReportMemUsgCmd::ReportMemUsgCmd(const std::string &name) : Cmd(name)
{
	optMgr_.setName(name);
	optMgr_.setShortDes("report resource usage");
	optMgr_.setDes("report resource usage");
	Opt *opt = new Opt(Opt::BOOL, "print usage", "");
	opt->addFlag("h");
	opt->addFlag("help");
	optMgr_.regOpt(opt);
}
ReportMemUsgCmd::~ReportMemUsgCmd() {}
//}}}
//{{{ bool ReportMemUsgCmd::exec()
bool ReportMemUsgCmd::exec(const vector<string> &argv)
{
	optMgr_.parse(argv);

	if (optMgr_.isFlagSet("h"))
	{
		optMgr_.usage();
		return true;
	}

	TmStat stat;
	TmUsage tmusg;
	tmusg.getPeriodUsage(stat);
	cout << "#  Memory         ";
	cout << "current " << (double)stat.vmSize / 1024.0 << " MB        ";
	cout << "peak " << (double)stat.vmPeak / 1024.0 << " MB" << endl;

	return true;
} //}}}
