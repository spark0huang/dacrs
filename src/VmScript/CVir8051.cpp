// ComDirver.cpp: implementation of the CComDirver class.
//
//////////////////////////////////////////////////////////////////////

#include "CVir8051.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hash.h"
#include "key.h"
#include "main.h"
#include <openssl/des.h>
#include <vector>
#include "VmScriptRun.h"
#include "tx.h"
//#include "Typedef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CVir8051::InitalReg() {

	memset(m_ChipRam, 0, sizeof(m_ChipRam));
	memset(m_ChipSfr, 0, sizeof(m_ChipSfr));
	memset(m_ExRam, 0, sizeof(m_ExRam));
//	memset(m_ChipRamoper, 0, sizeof(m_ChipRamoper));

	Sys.a.SetAddr(a_addr);
	Sys.b.SetAddr(b_addr);
	Sys.dptr.SetAddr(dptrl);
	Sys.psw.SetAddr(psw_addr);
	Sys.sp.SetAddr(sp_addr);

	Rges.R0.SetAddr(0x00);
	Rges.R1.SetAddr(0x01);
	Rges.R2.SetAddr(0x02);
	Rges.R3.SetAddr(0x03);
	Rges.R4.SetAddr(0x04);
	Rges.R5.SetAddr(0x05);
	Rges.R6.SetAddr(0x06);
	Rges.R7.SetAddr(0x07);

	SetRamData(0x80, 0xFF);
	SetRamData(0x90, 0xFF);
	SetRamData(0xa0, 0xFF);
	SetRamData(0xb0, 0xFF);
	Sys.sp = 0x07;

}

CVir8051::CVir8051(const vector<unsigned char> & vRom, const vector<unsigned char> &InputData) :
		Sys(this), Rges(this) {

	InitalReg();
	//INT16U addr = 0xFC00;

	memcpy(m_ExeFile, &vRom[0], vRom.size());
	unsigned char *ipara = (unsigned char *) GetExRamAddr(VM_SHARE_ADDR);
	int count = InputData.size();
	memcpy(ipara, &count, 2);
	memcpy(&ipara[2], &InputData[0],count);
}

CVir8051::~CVir8051() {

}

typedef tuple<bool,std::shared_ptr < std::vector< vector<unsigned char> > > > RET_DEFINE;
typedef tuple<bool,std::shared_ptr < std::vector< vector<unsigned char> > > > (*pFun)(unsigned char *,void *);

struct __MapExterFun {
	INT16U method;
	pFun fun;
};

static unsigned short GetParaLen(unsigned char * &pbuf) {
//	unsigned char tem[5];
	unsigned short ret = 0;
	memcpy(&ret, pbuf, 2);
	pbuf += 2;
//	memcpy(tem, pbuf, 5);
//	if (tem[0] < 0xFD) {
//		ret = tem[0];
//		pbuf += 1;
//	} else if (tem[0] == 0xFD) {
//		memcpy(&ret, &tem[1], 2);
//		pbuf += 3;
//	} else {
//		/// never come here
//	}
	return ret;
}

//static void GetParaData(unsigned char * &pbuf, unsigned char * &pdata, unsigned short datalen) {
//	pdata = pbuf;
//	pbuf += datalen;
//}
static bool GetData(unsigned char * ipara, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	int totallen = GetParaLen(ipara);
	//assert(totallen >= 0);
	if(totallen <= 0)
	{
		return false;
	}

	while (totallen > 0) {
		unsigned short length = GetParaLen(ipara);
		totallen -= (length + 2);
//		assert(totallen >= 0);
		if (length <=0) {
			return false;
		}
		ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(ipara, ipara + length));
		ipara += length;

//		ret.assign(std::make_shared<vector<unsigned char>>(ipara, ipara + length));
	}
	return true;
}
/**
 *COMP_RET Int64Compare(const Int64* const pM1, const Int64* const pM2)
 * �������ʽ���м�㴫��������������:
 * 1.��һ����int64_t���͵�����
 * 2.��һ����int64_t���͵�����
 */
static RET_DEFINE ExInt64CompFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	 if(!GetData(ipara,retdata) || retdata.size() != 2||
	    	retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
	    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int64_t m1, m2;
	unsigned char rslt;
	memcpy(&m1,  &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2,  &retdata.at(1).get()->at(0), sizeof(m2));

	if (m1 > m2) {
		rslt = 2;
	} else if (m1 == m2) {
		rslt = 0;
	} else {
		rslt = 1;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << rslt;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool Int64Mul(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * �������ʽ���м�㴫��������������:
 * 1.��һ����int64_t���͵�����
 * 2.��һ����int64_t���͵�����
 */
static RET_DEFINE ExInt64MullFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	 if(!GetData(ipara,retdata) ||retdata.size() != 2||
	        	retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
	    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }

	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 * m2;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool Int64Add(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * �������ʽ���м�㴫��������������:
 * 1.��һ����int64_t���͵�����
 * 2.��һ����int64_t���͵�����
 */
static RET_DEFINE ExInt64AddFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2||
        retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }

	int64_t m1, m2, m3;
	memcpy(&m1,  &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2,  &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 + m2;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool Int64Sub(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * �������ʽ���м�㴫��������������:
 * 1.��һ����int64_t���͵�����
 * 2.��һ����int64_t���͵�����
 */
static RET_DEFINE ExInt64SubFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2||
            retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 - m2;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool Int64Div(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * �������ʽ���м�㴫��������������:
 * 1.��һ����int64_t���͵�����
 * 2.��һ����int64_t���͵�����
 */
static RET_DEFINE ExInt64DivFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 2||
            retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if( m2 == 0)
	{
		return std::make_tuple (false, tem);
	}
	m3 = m1 / m2;
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool SHA256(void const* pfrist, const unsigned short len, void * const pout)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ����Ҫ������hashֵ���ַ���
 */
static RET_DEFINE ExSha256Func(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }

	uint256 rslt = Hash(&retdata.at(0).get()->at(0), &retdata.at(0).get()->at(0) + retdata.at(0).get()->size());

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << rslt;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *unsigned short Des(void const* pdata, unsigned short len, void const* pkey, unsigned short keylen, bool IsEn, void * const pOut,unsigned short poutlen)
 * �������ʽ���м�㴫����������������:
 * 1.��һ����Ҫ���������ݻ��߽�������
 * 2.�ڶ���ʽ���ܻ��߽��ܵ�keyֵ
 * 3.�����Ǳ�ʶ�����Ǽ��ܻ��ǽ���
 */
static RET_DEFINE ExDesFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 3)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }

	DES_key_schedule deskey1, deskey2, deskey3;

	vector<unsigned char> desdata;
	vector<unsigned char> desout;
	unsigned char datalen_rest = retdata.at(0).get()->size() % sizeof(DES_cblock);
	desdata.assign(retdata.at(0).get()->begin(), retdata.at(0).get()->end());
	if (datalen_rest) {
		desdata.insert(desdata.end(), sizeof(DES_cblock) - datalen_rest, 0);
	}

	const_DES_cblock in;
	DES_cblock out, key;

	desout.resize(desdata.size());

	unsigned char flag = retdata.at(2).get()->at(0);
	if (flag == 1) {
		if (retdata.at(1).get()->size() == 8) {
//			printf("the des encrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
//				printf("in :%s\r\n", HexStr(in, in + 8, true).c_str());
				DES_ecb_encrypt(&in, &out, &deskey1, DES_ENCRYPT);
//				printf("out :%s\r\n", HexStr(out, out + 8, true).c_str());
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}
		}
		else if(retdata.at(1).get()->size() == 16)
		{
//			printf("the 3 des encrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			DES_set_key_unchecked(&key, &deskey3);
			memcpy(key, &retdata.at(1).get()->at(0) + sizeof(DES_cblock), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey2);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
				DES_ecb3_encrypt(&in, &out, &deskey1, &deskey2, &deskey3, DES_ENCRYPT);
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}

		}
		else
		{
			//error
			auto tem =  make_shared<std::vector< vector<unsigned char> > >();
			return std::make_tuple (false, tem);
		}
	} else {
		if (retdata.at(1).get()->size() == 8) {
//			printf("the des decrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
//				printf("in :%s\r\n", HexStr(in, in + 8, true).c_str());
				DES_ecb_encrypt(&in, &out, &deskey1, DES_DECRYPT);
//				printf("out :%s\r\n", HexStr(out, out + 8, true).c_str());
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}
		}
		else if(retdata.at(1).get()->size() == 16)
		{
//			printf("the 3 des decrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			DES_set_key_unchecked(&key, &deskey3);
			memcpy(key, &retdata.at(1).get()->at(0) + sizeof(DES_cblock), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey2);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
				DES_ecb3_encrypt(&in, &out, &deskey1, &deskey2, &deskey3, DES_DECRYPT);
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}
		}
		else
		{
			//error
			auto tem =  make_shared<std::vector< vector<unsigned char> > >();
			return std::make_tuple (false, tem);
		}
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    (*tem.get()).push_back(desout);

	return std::make_tuple (true, tem);
}

/**
 *bool SignatureVerify(void const* data, unsigned short datalen, void const* key, unsigned short keylen,
		void const* phash, unsigned short hashlen)
 * �������ʽ���м�㴫����������������:
 * 1.��һ����ǩ��������
 * 2.�ڶ������õ�ǩ����publickey
 * 3.������ǩ��֮ǰ��hashֵ
 */
static RET_DEFINE ExVerifySignatureFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 3||
    		retdata.at(1).get()->size() != 33||
    		retdata.at(2).get()->size() !=32)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }

	CPubKey pk(retdata.at(1).get()->begin(),retdata.at(1).get()->end());
	uint256 hash(*retdata.at(2).get());

	bool rlt;
	if (pk.Verify(hash, *retdata.at(0))) {
		rlt = true;
	} else {
		rlt = false;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << rlt;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}

static RET_DEFINE ExSignatureFunc(unsigned char *ipara,void * pVmScriptRun) {
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true, tem);
}
/**
 *void LogPrint(const void *pdata, const unsigned short datalen,PRINT_FORMAT flag )
 * �������ʽ���м�㴫����������������:
 * 1.��һ���Ǵ�ӡ���ݵı�ʾ���ţ�true��һʮ�����ƴ�ӡ,�������ַ����ĸ�ʽ��ӡ
 * 2.�ڶ����Ǵ�ӡ���ַ���
 */
static RET_DEFINE ExLogPrintFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 2)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	CDataStream tep1(*retdata.at(0), SER_DISK, CLIENT_VERSION);
	bool flag ;
	tep1 >> flag;
	string pdata((*retdata[1]).begin(), (*retdata[1]).end());

	if(flag)
	{
		LogPrint("vm","%s\r\n", HexStr(pdata).c_str());
//		LogPrint("INFO","%s\r\n", HexStr(pdata).c_str());
	}else
	{
		LogPrint("vm","%s\r\n",pdata.c_str());
//		LogPrint("INFO","%s\r\n",pdata.c_str());
	}


	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true, tem);
}



/**
 *bool GetTxContacts(const unsigned char * const txhash,void* const pcotact,const unsigned short maxLen)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� hash
 */

static RET_DEFINE ExGetTxContractsFunc(unsigned char * ipara,void * pVmScriptRun) {

	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != 32)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	uint256 hash1(*retdata.at(0));
//	LogPrint("vm","ExGetTxContractsFunc1:%s\n",hash1.GetHex().c_str());


	std::shared_ptr<CBaseTransaction> pBaseTx;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();

	if (GetTransaction(pBaseTx, hash1)) {
		CContractTransaction *tx = static_cast<CContractTransaction*>(pBaseTx.get());
		 (*tem.get()).push_back(tx->vContract);

	}
	return std::make_tuple (true, tem);
}
/**
 *unsigned short GetAccounts(const unsigned char *txhash,void* const paccoutn,unsigned short maxlen)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� hash
 */

static RET_DEFINE ExGetTxAccountsFunc(unsigned char * ipara, void * pVmScriptRun) {
	vector<std::shared_ptr<vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 32)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	CDataStream tep1(*retdata.at(0), SER_DISK, CLIENT_VERSION);
	uint256 hash1(0);
	tep1 >>hash1;
//	LogPrint("vm","ExGetTxAccountsFunc:%s",hash1.GetHex().c_str());

	std::shared_ptr<CBaseTransaction> pBaseTx;

	auto tem = make_shared<std::vector<vector<unsigned char> > >();

	if (GetTransaction(pBaseTx, hash1)) {
		CContractTransaction *tx = static_cast<CContractTransaction*>(pBaseTx.get());
		vector<unsigned char> item;

		for (auto& it : tx->vAccountRegId) {
			vector<unsigned char> id = boost::get<CRegID>(it).GetVec6();
			item.insert(item.end(), id.begin(), id.end());
		}

		(*tem.get()).push_back(item);
	}
	return std::make_tuple(true, tem);
}
/**
 *unsigned short GetAccountPublickey(const void* const accounid,void * const pubkey,const unsigned short maxlength)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� �˻�id,�����ֽ�
 */
static RET_DEFINE ExGetAccountPublickeyFunc(unsigned char * ipara,void * pVmScriptRun) {
	CVmScriptRun *pVmScript = (CVmScriptRun *)pVmScriptRun;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 6)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
    bool flag = true;

	CAccount aAccount;
	CRegID regid(*retdata.at(0));
	CUserID userid(regid);
	if (!pVmScript->GetCatchView()->GetAccount(userid, aAccount)) {
		flag = false;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    vector<char> te;
    tep << aAccount.PublicKey;
    tep >>te;
    vector<unsigned char> tep1(te.begin(),te.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (flag, tem);
}
/**
 *bool QueryAccountBalance(const unsigned char* const account,Int64* const pBalance)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� �˻�id,�����ֽ�
 */
static RET_DEFINE ExQueryAccountBalanceFunc(unsigned char * ipara,void * pVmScriptRun) {
	CVmScriptRun *pVmScript = (CVmScriptRun *)pVmScriptRun;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 6)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	bool flag = true;
	CAccount aAccount;
	CRegID regid(*retdata.at(0));
	CUserID userid(regid);
	if (!pVmScript->GetCatchView()->GetAccount(userid, aAccount)) {
		flag = false;
	}
	uint64_t nbalance = aAccount.GetRawBalance(pVmScript->GetComfirHeight());
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << nbalance;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (flag , tem);
}
/**
 *unsigned long GetTxConFirmHeight(const void * const txhash)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� hash,�����ֽ�
 */
static RET_DEFINE ExGetTxConFirmHeightFunc(unsigned char * ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 32)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	uint256 hash1(*retdata.at(0));
//	LogPrint("vm","ExGetTxContractsFunc1:%s",hash1.GetHex().c_str());


	std::shared_ptr<CBaseTransaction> pBaseTx;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	int nHeight = GetTxComfirmHigh(hash1);
	if(-1 == nHeight)
	{
		std::make_tuple (false, tem);
	}

   CDataStream tep(SER_DISK, CLIENT_VERSION);
	tep << nHeight;
	vector<unsigned char> tep1(tep.begin(),tep.end());
	(*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);

}
static RET_DEFINE ExGetTipHeightFunc(unsigned char * ipara,void * pVmScriptRun) {
	int height = chainActive.Height();
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << height;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool GetBlockHash(const unsigned long height,void * const pblochHash)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� int���͵Ĳ���
 */
static RET_DEFINE ExGetBlockHashFunc(unsigned char * ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
	CVmScriptRun *pVmScript = (CVmScriptRun *)pVmScriptRun;
    if(!GetData(ipara,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != sizeof(int))
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int height = 0;
	memcpy(&height, &retdata.at(0).get()->at(0), sizeof(int));
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if (height <= 0 || height >= pVmScript->GetComfirHeight()) //��ǰblock �ǲ����Ի�ȡhash��
	{
		return std::make_tuple (false, tem);
	}

	if(chainActive.Height() < height){	         //��ȡ�ȵ�ǰ�߶ȸߵ������ǲ����Ե�
		return std::make_tuple (false, tem);
	}
	CBlockIndex *pindex = chainActive[height];
	uint256 blockHash = pindex->GetBlockHash();


//	LogPrint("vm","ExGetBlockHashFunc:%s",HexStr(blockHash).c_str());
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << blockHash;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);

}

static RET_DEFINE ExGetCurRunEnvHeightFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	int height = pVmScriptRun->GetComfirHeight();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << height;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *bool WriteDataDB(const void* const key,const unsigned char keylen,const void * const value,const unsigned short valuelen,const unsigned long time)
 * �������ʽ���м�㴫����������������:
 * 1.��һ���� keyֵ
 * 2.�ڶ�����valueֵ
 * 3.�������ǳ�ʱ�߶�
 */
static RET_DEFINE ExWriteDataDBFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 3)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int height = 0;
	memcpy(&height,&retdata.at(2).get()->at(0),4);

	const CRegID scriptid = pVmScriptRun->GetScriptRegID();
	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();

	CScriptDBOperLog operlog;
	if (!scriptDB->SetScriptData(scriptid, *retdata.at(0), *retdata.at(1), height, operlog)) {
		flag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
		(*m_dblog.get()).push_back(operlog);
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);
	return std::make_tuple (true, tem);
}
/**
 *bool DeleteDataDB(const void* const key,const unsigned char keylen)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� keyֵ
 */
static RET_DEFINE ExDeleteDataDBFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	CRegID scriptid = pVmScriptRun->GetScriptRegID();

	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();

	CScriptDBOperLog operlog;
	if (!scriptDB->EraseScriptData(scriptid, *retdata.at(0), operlog)) {
		flag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
		m_dblog.get()->push_back(operlog);
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true, tem);
}
/**
 *unsigned short ReadDataValueDB(const void* const key,const unsigned char keylen, void* const value,unsigned short const maxbuffer)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� keyֵ
 */
static RET_DEFINE ExReadDataValueDBFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	CRegID scriptid = pVmScriptRun->GetScriptRegID();

	vector_unsigned_char vValue;
	int nHeight;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();
	bool flag =true;

//	LogPrint("INFO", "script run read data:%s\n", HexStr(*retdata.at(0)));
	CScriptDBOperLog operLog;
	if(!scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid, *retdata.at(0), vValue, nHeight, operLog))
	{
		if(!operLog.vKey.empty()) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operLog);
		}
		flag = false;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    (*tem.get()).push_back(vValue);

	return std::make_tuple (flag, tem);
}
/**
 *bool ModifyDataDB(const void* const key,const unsigned char keylen, const void* const pvalue,const unsigned short valuelen,const unsigned long ptime)
 * �������ʽ���м�㴫����������������:
 * 1.��һ���� keyֵ
 * 2.�ڶ�����valueֵ
 * 3.�������ǳ�ʱ�߶�
 */
static RET_DEFINE ExModifyDataDBFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 3)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int height = 0;
	memcpy(&height,&retdata.at(2).get()->at(0),4);

	const CRegID scriptid = pVmScriptRun->GetScriptRegID();
	bool flag = false;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();

	CScriptDBOperLog operlog;
	vector_unsigned_char vTemp;
	int nHeight;
	if(scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid, *retdata.at(0), vTemp, nHeight, operlog)) {
		if(scriptDB->SetScriptData(scriptid,*retdata.at(0),*retdata.at(1).get(),height,operlog))
		{
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operlog);
			flag = true;
		}
	}else {
		if(!operlog.vKey.empty()) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operlog);
		}
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);
	return std::make_tuple (true, tem);
}

static RET_DEFINE ExGetDBSizeFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	CRegID scriptid = pVmScriptRun->GetScriptRegID();
	vector<unsigned char> vScriptKey;
	int count = 0;
	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();
	if(!scriptDB->GetScriptDataCount(scriptid,count))
	{
		flag = false;
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << count;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (flag, tem);
}
/**
 *bool GetDBValue(const unsigned long index,void* const key,unsigned char * const keylen,unsigned short maxkeylen,void* const value,unsigned short* const maxbuffer, unsigned long* const ptime)
 * �����ĵ�һ������index == 0������һ����������
 * 1.��һ���� indexֵ
 * �����ĵ�һ������index == 1��������������������
 * 1.��һ���� indexֵ
 * 2.�ڶ���keyֵ
 */
static RET_DEFINE ExGetDBValueFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||(retdata.size() != 2 && retdata.size() != 1))
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int index = 0;
	bool flag = true;
	memcpy(&index,&retdata.at(0).get()->at(0),sizeof(int));
	if(!(index == 0 ||(index == 1 && retdata.size() == 2)))
	{
		flag =  false;
	}
	CRegID scriptid = pVmScriptRun->GetScriptRegID();

	vector_unsigned_char vValue;
	int nHeight;
	vector<unsigned char> vScriptKey;
	if(index == 1)
	{
		vScriptKey.assign(retdata.at(1).get()->begin(),retdata.at(1).get()->end());
	}

	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();
	set<CScriptDBOperLog> setOperLog;
	flag = scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid,index,vScriptKey,vValue,nHeight,setOperLog);

	if (!setOperLog.empty()) {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
		for(auto &item : setOperLog)
			m_dblog.get()->push_back(item);
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    (*tem.get()).push_back(vScriptKey);
	(*tem.get()).push_back(vValue);
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << nHeight;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);
	return std::make_tuple (flag, tem);
}
static RET_DEFINE ExGetCurTxHash(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	uint256 hash = pVmScriptRun->GetCurTxHash();
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << hash;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

 //   LogPrint("vm","ExGetCurTxHash:%s",HexStr(hash).c_str());
	return std::make_tuple (true, tem);
}
/**
 *bool IsAuthorited(const void* const account,const Int64* const pmoney)
 * �м�㴫����������
 * 1.��һ���� �˺ŵ�id
 * 2.�ڶ�����int64_t���͵�����
 */
static RET_DEFINE ExIsAuthoritFunc(unsigned char * ipara,void * pVmScript) {
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2
    		|| retdata.at(0).get()->size() != 6
    		|| retdata.at(1).get()->size() != sizeof(uint64_t))
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	uint64_t money;
	memcpy(&money,&retdata.at(1).get()->at(0),sizeof(money));

	bool flag = true;
	CAccount aAccount;
	CRegID regid(*retdata.at(0));
	CUserID userid(regid);
	if (!pVmScriptRun->GetCatchView()->GetAccount(userid, aAccount)) {
		flag = false;
	}

	CRegID scriptid = pVmScriptRun->GetScriptRegID();
	int height = pVmScriptRun->GetComfirHeight();
	bool ret = aAccount.IsAuthorized(money,height,scriptid.GetVec6());

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << ret;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (flag, tem);

}
/**
 *bool ReadDataDBTime(const void* const key,const unsigned char keylen, unsigned long * const ptime)
 * �м�㴫��һ��������
 * 1.��һ���� key
 */
static RET_DEFINE ExReadDataDBTimeFunc(unsigned char * ipara,void * pVmScript)
{
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1 )
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	CRegID scriptid = pVmScriptRun->GetScriptRegID();
	vector_unsigned_char vValue;
	int nHeight;
	bool flag = true;
	CScriptDBViewCache *scriptDB = pVmScriptRun->GetScriptDB();

	CScriptDBOperLog operLog;
	if(!scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid,*retdata.at(0),vValue,nHeight, operLog))
	{
		if(!operLog.vKey.empty()) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operLog);
		}
		flag =  false;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << nHeight;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (flag, tem);
}
/**
 *bool ModifyDataDBTime(const void* const key,const unsigned char keylen, const unsigned long ptime)
 * �м�㴫����������
 * 1.��һ���� key
 * 2.�ڶ����� ��ʱʱ��
 */
static RET_DEFINE ExModifyDataDBTimeFunc(unsigned char * ipara,void * pVmScript)
{
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2 )
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	int height = 0;
	memcpy(&height,&retdata.at(1).get()->at(0),4);

	CRegID scriptid = pVmScriptRun->GetScriptRegID();
	vector_unsigned_char vValue;
	bool flag = false;
	bool ret = true;
	int temp = 0;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();

	CScriptDBOperLog operlog;
	if(scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid,*retdata.at(0),vValue,temp,operlog))
	{
		if(scriptDB->SetScriptData(scriptid,*retdata.at(0),vValue,height,operlog))
		{
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operlog);
			flag = true;
		}
	}else {
		if (!operlog.vKey.empty()) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operlog);
		}
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (ret , tem);
}
/**
 *bool ModifyDataDBVavle(const void* const key,const unsigned char keylen, const void* const pvalue,const unsigned short valuelen)
 * �м�㴫����������
 * 1.��һ���� key
 * 2.�ڶ����� value
 */
static RET_DEFINE ExModifyDataDBVavleFunc(unsigned char * ipara,void * pVmScript)
{
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;

	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2 )
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	CRegID scriptid = pVmScriptRun->GetScriptRegID();
	vector_unsigned_char vValue;
	bool flag = false;
	int temp = 0;
	bool ret = true;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();

	CScriptDBOperLog operlog;
	if(scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid,*retdata.at(0),vValue,temp,operlog))
	{
		if(scriptDB->SetScriptData(scriptid,*retdata.at(0),*retdata.at(1),temp,operlog))
		{
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operlog);
			flag = true;
		}
	}else {
		if (!operlog.vKey.empty()) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operlog);
		}
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (ret , tem);
}
/**
 *bool WriteOutput( const VM_OPERATE* data, const unsigned short conter)
 * �м�㴫��һ������
 * 1.��һ�������ָ��
 */
static RET_DEFINE ExWriteOutputFunc(unsigned char * ipara,void * pVmScript)
{

	unsigned char * pbuffer = ipara;
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1 )
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	vector<CVmOperate> source;
	CVmOperate temp;
	int Size = ::GetSerializeSize(temp, SER_NETWORK, PROTOCOL_VERSION);
	int count = GetParaLen(pbuffer)/Size;
	CDataStream ss(*retdata.at(0),SER_DISK, CLIENT_VERSION);

	while(count--)
	{
		ss >> temp;
      source.push_back(temp);
	}
	pVmScriptRun->InsertOutputData(source);
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true , tem);
}
/**
 *unsigned short GetAuthUserDefine(const void* const account,void *const pout,const unsigned short maxlen)
 * �м�㴫��һ������
 * 1.�˻���id
 */
static RET_DEFINE ExGetAuthoritedDefineFunc(unsigned char * ipara,void * pVmScript){
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != 6)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	bool flag = true;
	CAccount aAccount;
	CRegID regid(*retdata.at(0));
	CUserID userid(regid);
	if (!pVmScriptRun->GetCatchView()->GetAccount(userid, aAccount)) {
		flag = false;
	}

	vector_unsigned_char scriptid = pVmScriptRun->GetScriptRegID().GetVec6();
//	int height = pVmScriptRun->GetComfirHeight();

	vector<unsigned char> vData;
	if(!aAccount.GetUserData(scriptid,vData))
	{
		flag = false;
	}

	/// untodo
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
   (*tem.get()).push_back(vData);

	return std::make_tuple (flag, tem);
}
/**
 *bool GetScriptData(const void* const scriptID,void* const pkey,short len,void* const pvalve,short maxlen)
 * �м�㴫������������
 * 1.�ű���id��
 * 2.���ݿ��keyֵ
 */
static RET_DEFINE ExGetScriptDataFunc(unsigned char * ipara,void * pVmScript)
{
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2 || retdata.at(0).get()->size() != 6)
    {
    	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    	return std::make_tuple (false, tem);
    }
	vector_unsigned_char vValue;
	int nHeight;
	bool flag =true;
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	CScriptDBViewCache* scriptDB = pVmScriptRun->GetScriptDB();
	CRegID scriptid(*retdata.at(0));
	CScriptDBOperLog operLog;
	if(!scriptDB->GetScriptData(pVmScriptRun->GetComfirHeight(),scriptid, *retdata.at(1), vValue, nHeight, operLog))
	{
		if (!operLog.vKey.empty()) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmScriptRun->GetDbLog();
			m_dblog.get()->push_back(operLog);
		}
		flag = false;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    (*tem.get()).push_back(vValue);

	return std::make_tuple (flag, tem);

}
static RET_DEFINE ExGetScriptIDFunc(unsigned char * ipara,void * pVmScript)
{
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;

	vector_unsigned_char scriptid = pVmScriptRun->GetScriptRegID().GetVec6();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
   (*tem.get()).push_back(scriptid);

	return std::make_tuple (true, tem);
}
static RET_DEFINE ExGetCurTxAccountFunc(unsigned char * ipara,void * pVmScript)
{
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<CUserID> regid =pVmScriptRun->GetTxAccount();

	vector<unsigned char> item;
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
		for (auto& it : regid) {
			vector<unsigned char> id = boost::get<CRegID>(it).GetVec6();
			item.insert(item.end(), id.begin(), id.end());
		}

		(*tem.get()).push_back(item);
		return std::make_tuple (true, tem);
}
static RET_DEFINE ExGetCurTxContactFunc(unsigned char * ipara,void * pVmScript)
{
	CVmScriptRun *pVmScriptRun = (CVmScriptRun *)pVmScript;
	vector<unsigned char> contact =pVmScriptRun->GetTxContact();

	vector<unsigned char> item;
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();

	(*tem.get()).push_back(contact);
	return std::make_tuple (true, tem);
}
enum CALL_API_FUN {
	COMP_FUNC = 0,            //!< COMP_FUNC
	MULL_MONEY ,              //!< MULL_MONEY
	ADD_MONEY ,               //!< ADD_MONEY
	SUB_MONEY,                //!< SUB_MONEY
	DIV_MONEY,                //!< DIV_MONEY
	SHA256_FUNC,              //!< SHA256_FUNC
	DES_FUNC,                 //!< DES_FUNC
	VERFIY_SIGNATURE_FUNC,    //!< VERFIY_SIGNATURE_FUNC
	SIGNATURE_FUNC,           //!< SIGNATURE_FUNC
	PRINT_FUNC,               //!< PRINT_FUNC
	GETTX_CONTRACT_FUNC,      //!< GETTX_CONTRACT_FUNC
	GETTX_ACCOUNT_FUNC,       //!< GETTX_ACCOUNT_FUNC
	GETACCPUB_FUNC,           //!< GETACCPUB_FUNC
	QUEYACCBALANCE_FUNC,      //!< QUEYACCBALANCE_FUNC
	GETTXCONFIRH_FUNC,        //!< GETTXCONFIRH_FUNC
	GETTIPH_FUNC,             //!< GETTIPH_FUNC
	GETBLOCKHASH_FUNC,        //!< GETBLOCKHASH_FUNC
	ISAUTHORIT_FUNC,          //!<ISAUTHORIT
	GETAUTHORITDEFINE_FUNC,   //!GETAUTHORITDEFINE_FUNC


	//// tx api
	GETCTXCONFIRMH_FUNC,//!< GETCTXCONFIRMH_FUNC
	WRITEDB_FUNC,       //!< WRITEDB_FUNC
	DELETEDB_FUNC,      //!< DELETEDB_FUNC
	READDB_FUNC,        //!< READDB_FUNC
	MODIFYDB_FUNC,      //!< MODIFYDB_FUNC
	GETDBSIZE_FUNC,     //!< GETDBSIZE_FUNC
	GETDBVALUE_FUNC,    //!< GETDBVALUE_FUNC
	GetCURTXHASH_FUNC,  //!< GetCURTXHASH_FUNC
	READDBTIME_FUNC,     //!< READDBTIME_FUNC
	MODIFYDBTIME_FUNC,  //!< MODIFYDBTIME_FUNC
	MODIFYDBVALUE_FUNC ,  //!< MODIFYDBVALUE_FUNC
	WRITEOUTPUT_FUNC,     //!<WRITEOUTPUT_FUNC

	GETSCRIPTDATA_FUNC,		  //!<GETSCRIPTDATA_FUNC
	GETSCRIPTID_FUNC,		//!<GETSCRIPTID_FUNC
	GETCURTXACCOUNT_FUNC,//!<GETCURTXACCOUNT_FUNC
	GETCURTXCONTACT_FUNC,		 //!<GETCURTXCONTACT_FUNC
};

const static struct __MapExterFun FunMap[] = { //
		{ COMP_FUNC, ExInt64CompFunc },			//
		{ MULL_MONEY, ExInt64MullFunc },			//
		{ ADD_MONEY, ExInt64AddFunc },			//
		{ SUB_MONEY, ExInt64SubFunc },			//
		{ DIV_MONEY, ExInt64DivFunc },			//
		{ SHA256_FUNC, ExSha256Func },			//
		{ DES_FUNC, ExDesFunc },			    //
		{ VERFIY_SIGNATURE_FUNC, ExVerifySignatureFunc },   //
		{ SIGNATURE_FUNC, ExSignatureFunc },			//
		{ PRINT_FUNC, ExLogPrintFunc },         //
		{GETTX_CONTRACT_FUNC,ExGetTxContractsFunc},            //
		{GETTX_ACCOUNT_FUNC,ExGetTxAccountsFunc},
		{GETACCPUB_FUNC,ExGetAccountPublickeyFunc},
		{QUEYACCBALANCE_FUNC,ExQueryAccountBalanceFunc},
		{GETTXCONFIRH_FUNC,ExGetTxConFirmHeightFunc},
		{GETTIPH_FUNC,ExGetTipHeightFunc},
		{GETBLOCKHASH_FUNC,ExGetBlockHashFunc},
		{ISAUTHORIT_FUNC,ExIsAuthoritFunc},
		{GETAUTHORITDEFINE_FUNC,ExGetAuthoritedDefineFunc},

		{GETCTXCONFIRMH_FUNC,ExGetCurRunEnvHeightFunc},
		{WRITEDB_FUNC,ExWriteDataDBFunc},
		{DELETEDB_FUNC,ExDeleteDataDBFunc},
		{READDB_FUNC,ExReadDataValueDBFunc},
		{MODIFYDB_FUNC,ExModifyDataDBFunc},
		{GETDBSIZE_FUNC,ExGetDBSizeFunc},
		{GETDBVALUE_FUNC,ExGetDBValueFunc},
		{GetCURTXHASH_FUNC,ExGetCurTxHash},
		{READDBTIME_FUNC,ExReadDataDBTimeFunc},
		{MODIFYDBTIME_FUNC,ExModifyDataDBTimeFunc},
		{MODIFYDBVALUE_FUNC,ExModifyDataDBVavleFunc},
		{WRITEOUTPUT_FUNC,ExWriteOutputFunc},
		{GETSCRIPTDATA_FUNC,ExGetScriptDataFunc},
		{GETSCRIPTID_FUNC,ExGetScriptIDFunc},
		{GETCURTXACCOUNT_FUNC,ExGetCurTxAccountFunc	  },
		{GETCURTXCONTACT_FUNC,ExGetCurTxContactFunc		},

		};

RET_DEFINE CallExternalFunc(INT16U method, unsigned char *ipara,CVmScriptRun *pVmScriptRun) {
	return FunMap[method].fun(ipara,pVmScriptRun);

}

int64_t CVir8051::run(uint64_t maxstep,CVmScriptRun *pVmScriptRun) {
	INT8U code = 0;
	uint64_t step = 0;

	while (1) {
		code = GetOpcode();
		StepRun(code);
		step++;
		//call func out of 8051
		if (Sys.PC == 0x0012) {
			//get what func will be called
			INT16U methodID = ((INT16U) GetExRam(VM_FUN_CALL_ADDR) | ((INT16U) GetExRam(VM_FUN_CALL_ADDR+1) << 8));
			unsigned char *ipara = (unsigned char *) GetExRamAddr(VM_SHARE_ADDR);		//input para
			RET_DEFINE retdata = CallExternalFunc(methodID, ipara, pVmScriptRun);
			memset(ipara, 0, MAX_SHARE_RAM);

			if (std::get<0>(retdata)) {
				auto tem = std::get<1>(retdata);
				int pos = 0;
				int totalsize = 0;
				for (auto& it : *tem.get()) {
					totalsize += it.size() + 2;
				}
				if (totalsize + 2 < MAX_SHARE_RAM) { //if data not over
					for (auto& it : *tem.get()) {
						int size = it.size();
//						if(methodID == READDB_FUNC)
//							LogPrint("vm","size:%d",size);
						memcpy(&ipara[pos], &size, 2);
						memcpy(&ipara[pos + 2], &it.at(0), size);
						pos += size + 2;
					}
				}
//				if(methodID == READDB_FUNC) {
//					LogPrint("vm","data1:%s\r\n",HexStr(ipara,ipara+totalsize));
//					LogPrint("INFO","data1:%s\r\n",HexStr(ipara,ipara+totalsize));
//				}
			}
		} else if (Sys.PC == 0x0008) {
				INT8U result=GetExRam(0xEFFD);
				if(result == 0x01)
				{
					return step;
				}
				return 0;
			}
//		//// for test
//			if (maxstep != 0 && step > maxstep) {
//
//				return -1;		//force return
//			}
	}

	return 1;
}

bool CVir8051::run() {

	INT8U code = 0;
//	INT16U flag;
	while (1) {
		code = GetOpcode();
		StepRun(code);
//		UpDataDebugInfo();

		//call func out of 8051
		if (Sys.PC == 0x0012) {
			//get what func will be called
			INT16U method = ((INT16U) GetExRam(VM_FUN_CALL_ADDR) | ((INT16U) GetExRam(VM_FUN_CALL_ADDR+1) << 8));
//			flag = method;
			unsigned char *ipara = (unsigned char *) GetExRamAddr(VM_SHARE_ADDR);		//input para
			CVmScriptRun *pVmScript = NULL;
			RET_DEFINE retdata = CallExternalFunc(method, ipara, pVmScript);
			//memset(ipara, 0, MAX_SHARE_RAM);
			memset(ipara, 0, 8);
			if (std::get<0>(retdata)) {
				auto tem = std::get<1>(retdata);
				int pos = 0;
				int totalsize = 0;
				for (auto& it : *tem.get()) {
					totalsize += it.size() + 2;
				}
				if (totalsize + 2 < MAX_SHARE_RAM) {
					for (auto& it : *tem.get()) {
						int size = it.size();
						memcpy(&ipara[pos], &size, 2);
						memcpy(&ipara[pos + 2], &it.at(0), size);
						pos += size + 2;
					}
				}

			}
		}
		if (Sys.PC == 0x0008) {
			{
				INT8U result=GetExRam(0xEFFD);
				if(result == 0x01)
				{
					return 1;
				}
				return 0;
			}
		}
	}

	return 1;
}
void CVir8051::SetExRamData(INT16U addr, const vector<unsigned char> data) {
	memcpy(&m_ExRam[addr], &data[0], data.size());
}
void CVir8051::StepRun(INT8U code) {
//		INT8U tempa = Sys.a();
//	m_ChipRamoper[code] = 1;
//	char temp[1024];
//	sprintf(temp, "pc--->%x,code:%x \r\n", Sys.PC, code);
//	cout << "before" << temp << endl;
	switch (code) {
	case 0x00: {
		Opcode_00_NOP();
		break;
	}
	case 0x01: {
		Opcode_01_AJMP_Addr11();
		break;
	}
	case 0x02: {
		Opcode_02_LJMP_Addr16();
		break;
	}
	case 0x03: {
		Opcode_03_RR_A();
		break;
	}
	case 0x04: {
		Opcode_04_INC_A();
		break;
	}
	case 0x05: {
		Opcode_05_INC_Direct();
		break;
	}
	case 0x06: {
		Opcode_06_INC_R0_1();
		break;
	}
	case 0x07: {
		Opcode_07_INC_R1_1();
		break;
	}
	case 0x08: {
		Opcode_08_INC_R0();
		break;
	}
	case 0x09: {
		Opcode_09_INC_R1();
		break;
	}
	case 0x0A: {
		Opcode_0A_INC_R2();
		break;
	}
	case 0x0B: {
		Opcode_0B_INC_R3();
		break;
	}
	case 0x0C: {
		Opcode_0C_INC_R4();
		break;
	}
	case 0x0D: {
		Opcode_0D_INC_R5();
		break;
	}
	case 0x0E: {
		Opcode_0E_INC_R6();
		break;
	}
	case 0x0F: {
		Opcode_0F_INC_R7();
		break;
	}
	case 0x10: {
		Opcode_10_JBC_Bit_Rel();
		break;
	}
	case 0x11: {
		Opcode_11_ACALL_Addr11();
		break;
	}
	case 0x12: {
		Opcode_12_LCALL_Addr16();
		break;
	}
	case 0x13: {
		Opcode_13_RRC_A();
		break;
	}
	case 0x14: {
		Opcode_14_DEC_A();
		break;
	}
	case 0x15: {
		Opcode_15_DEC_Direct();
		break;
	}
	case 0x16: {
		Opcode_16_DEC_R0_1();
		break;
	}
	case 0x17: {
		Opcode_17_DEC_R1_1();
		break;
	}
	case 0x18: {
		Opcode_18_DEC_R0();
		break;
	}
	case 0x19: {
		Opcode_19_DEC_R1();
		break;
	}
	case 0x1A: {
		Opcode_1A_DEC_R2();
		break;
	}
	case 0x1B: {
		Opcode_1B_DEC_R3();
		break;
	}
	case 0x1C: {
		Opcode_1C_DEC_R4();
		break;
	}
	case 0x1D: {
		Opcode_1D_DEC_R5();
		break;
	}
	case 0x1E: {
		Opcode_1E_DEC_R6();
		break;
	}
	case 0x1F: {
		Opcode_1F_DEC_R7();
		break;
	}
	case 0x20: {
		Opcode_20_JB_Bit_Rel();
		break;
	}
	case 0x21: {
		Opcode_21_AJMP_Addr11();
		break;
	}
	case 0x22: {
		Opcode_22_RET();
		break;
	}
	case 0x23: {
		Opcode_23_RL_A();
		break;
	}
	case 0x24: {
		Opcode_24_ADD_A_Data();
		break;
	}
	case 0x25: {
		Opcode_25_ADD_A_Direct();
		break;
	}
	case 0x26: {
		Opcode_26_ADD_A_R0_1();
		break;
	}
	case 0x27: {
		Opcode_27_ADD_A_R1_1();
		break;
	}
	case 0x28: {
		Opcode_28_ADD_A_R0();
		break;
	}
	case 0x29: {
		Opcode_29_ADD_A_R1();
		break;
	}
	case 0x2A: {
		Opcode_2A_ADD_A_R2();
		break;
	}
	case 0x2B: {
		Opcode_2B_ADD_A_R3();
		break;
	}
	case 0x2C: {
		Opcode_2C_ADD_A_R4();
		break;
	}
	case 0x2D: {
		Opcode_2D_ADD_A_R5();
		break;
	}
	case 0x2E: {
		Opcode_2E_ADD_A_R6();
		break;
	}
	case 0x2F: {
		Opcode_2F_ADD_A_R7();
		break;
	}
	case 0x30: {
		Opcode_30_JNB_Bit_Rel();
		break;
	}
	case 0x31: {
		Opcode_31_ACALL_Addr11();
		break;
	}
	case 0x32: {
		Opcode_32_RETI();
		break;
	}
		/*�����Ѿ�����PASS*/
	case 0x33: {
		Opcode_33_RLC_A();
		break;
	}
	case 0x34: {
		Opcode_34_ADDC_A_Data();
		break;
	}
	case 0x35: {
		Opcode_35_ADDC_A_Direct();
		break;
	}
	case 0x36: {
		Opcode_36_ADDC_A_R0_1();
		break;
	}
	case 0x37: {
		Opcode_37_ADDC_A_R1_1();
		break;
	}
	case 0x38: {
		Opcode_38_ADDC_A_R0();
		break;
	}
	case 0x39: {
		Opcode_39_ADDC_A_R1();
		break;
	}
	case 0x3A: {
		Opcode_3A_ADDC_A_R2();
		break;
	}
	case 0x3B: {
		Opcode_3B_ADDC_A_R3();
		break;
	}
	case 0x3C: {
		Opcode_3C_ADDC_A_R4();
		break;
	}
	case 0x3D: {
		Opcode_3D_ADDC_A_R5();
		break;
	}
	case 0x3E: {
		Opcode_3E_ADDC_A_R6();
		break;
	}
	case 0x3F: {
		Opcode_3F_ADDC_A_R7();
		break;
	}
		/*�����Ѿ�����PASS*/
	case 0x40: {
		Opcode_40_JC_Rel();
		break;
	}
	case 0x41: {
		Opcode_41_AJMP_Addr11();
		break;
	}
	case 0x42: {
		Opcode_42_ORL_Direct_A();
		break;
	}
	case 0x43: {
		Opcode_43_ORL_Direct_Data();
		break;
	}
	case 0x44: {
		Opcode_44_ORL_A_Data();
		break;
	}
	case 0x45: {
		Opcode_45_ORL_A_Direct();
		break;
	}
	case 0x46: {
		Opcode_46_ORL_A_R0_1();
		break;
	}
	case 0x47: {
		Opcode_47_ORL_A_R1_1();
		break;
	}
	case 0x48: {
		Opcode_48_ORL_A_R0();
		break;
	}
	case 0x49: {
		Opcode_49_ORL_A_R1();
		break;
	}
	case 0x4A: {
		Opcode_4A_ORL_A_R2();
		break;
	}
	case 0x4B: {
		Opcode_4B_ORL_A_R3();
		break;
	}
	case 0x4C: {
		Opcode_4C_ORL_A_R4();
		break;
	}
	case 0x4D: {
		Opcode_4D_ORL_A_R5();
		break;
	}
	case 0x4E: {
		Opcode_4E_ORL_A_R6();
		break;
	}
	case 0x4F: {
		Opcode_4F_ORL_A_R7();
		break;
	}
	case 0x50: {
		Opcode_50_JNC_Rel();
		break;
	}
	case 0x51: {
		Opcode_51_ACALL_Addr11();
		break;
	}
	case 0x52: {
		Opcode_52_ANL_Direct_A();
		break;
	}
	case 0x53: {
		Opcode_53_ANL_Direct_Data();
		break;
	}
	case 0x54: {
		Opcode_54_ANL_A_Data();
		break;
	}
	case 0x55: {
		Opcode_55_ANL_A_Direct();
		break;
	}
	case 0x56: {
		Opcode_56_ANL_A_R0_1();
		break;
	}
	case 0x57: {
		Opcode_57_ANL_A_R1_1();
		break;
	}
	case 0x58: {
		Opcode_58_ANL_A_R0();
		break;
	}
	case 0x59: {
		Opcode_59_ANL_A_R1();
		break;
	}
	case 0x5A: {
		Opcode_5A_ANL_A_R2();
		break;
	}
	case 0x5B: {
		Opcode_5B_ANL_A_R3();
		break;
	}
	case 0x5C: {
		Opcode_5C_ANL_A_R4();
		break;
	}
	case 0x5D: {
		Opcode_5D_ANL_A_R5();
		break;
	}
	case 0x5E: {
		Opcode_5E_ANL_A_R6();
		break;
	}
	case 0x5F: {
		Opcode_5F_ANL_A_R7();
		break;
	}
	case 0x60: {
		Opcode_60_JZ_Rel();
		break;
	}
	case 0x61: {
		Opcode_61_AJMP_Addr11();
		break;
	}
	case 0x62: {
		Opcode_62_XRL_Direct_A();
		break;
	}
	case 0x63: {
		Opcode_63_XRL_Direct_Data();
		break;
	}
	case 0x64: {
		Opcode_64_XRL_A_Data();
		break;
	}
	case 0x65: {
		Opcode_65_XRL_A_Direct();
		break;
	}
	case 0x66: {
		Opcode_66_XRL_A_R0_1();
		break;
	}
	case 0x67: {
		Opcode_67_XRL_A_R1_1();
		break;
	}
	case 0x68: {
		Opcode_68_XRL_A_R0();
		break;
	}
	case 0x69: {
		Opcode_69_XRL_A_R1();
		break;
	}
	case 0x6A: {
		Opcode_6A_XRL_A_R2();
		break;
	}
	case 0x6B: {
		Opcode_6B_XRL_A_R3();
		break;
	}
	case 0x6C: {
		Opcode_6C_XRL_A_R4();
		break;
	}
	case 0x6D: {
		Opcode_6D_XRL_A_R5();
		break;
	}
	case 0x6E: {
		Opcode_6E_XRL_A_R6();
		break;
	}
	case 0x6F: {
		Opcode_6F_XRL_A_R7();
		break;
	}
	case 0x70: {
		Opcode_70_JNZ_Rel();
		break;
	}
	case 0x71: {
		Opcode_71_ACALL_Addr11();
		break;
	}
	case 0x72: {
		Opcode_72_ORL_C_Direct();
		break;
	}
	case 0x73: {
		Opcode_73_JMP_A_DPTR();
		break;
	}
	case 0x74: {
		Opcode_74_MOV_A_Data();
		break;
	}
	case 0x75: {
		Opcode_75_MOV_Direct_Data();
		break;
	}
	case 0x76: {
		Opcode_76_MOV_R0_1_Data();
		break;
	}
	case 0x77: {
		Opcode_77_MOV_R1_1_Data();
		break;
	}
	case 0x78: {
		Opcode_78_MOV_R0_Data();
		break;
	}
	case 0x79: {
		Opcode_79_MOV_R1_Data();
		break;
	}
	case 0x7A: {
		Opcode_7A_MOV_R2_Data();
		break;
	}
	case 0x7B: {
		Opcode_7B_MOV_R3_Data();
		break;
	}
	case 0x7C: {
		Opcode_7C_MOV_R4_Data();
		break;
	}
	case 0x7D: {
		Opcode_7D_MOV_R5_Data();
		break;
	}
	case 0x7E: {
		Opcode_7E_MOV_R6_Data();
		break;
	}
	case 0x7F: {
		Opcode_7F_MOV_R7_Data();
		break;
	}
	case 0x80: {
		Opcode_80_SJMP_Rel();
		break;
	}
	case 0x81: {
		Opcode_81_AJMP_Addr11();
		break;
	}
	case 0x82: {
		Opcode_82_ANL_C_Bit();
		break;
	}
	case 0x83: {
		Opcode_83_MOVC_A_PC();
		break;
	}
	case 0x84: {
		Opcode_84_DIV_AB();
		break;
	}
	case 0x85: {
		Opcode_85_MOV_Direct_Direct();
		break;
	}
	case 0x86: {
		Opcode_86_MOV_Direct_R0_1();
		break;
	}
	case 0x87: {
		Opcode_87_MOV_Direct_R1_1();
		break;
	}
	case 0x88: {
		Opcode_88_MOV_Direct_R0();
		break;
	}
	case 0x89: {
		Opcode_89_MOV_Direct_R1();
		break;
	}
	case 0x8A: {
		Opcode_8A_MOV_Direct_R2();
		break;
	}
	case 0x8B: {
		Opcode_8B_MOV_Direct_R3();
		break;
	}
	case 0x8C: {
		Opcode_8C_MOV_Direct_R4();
		break;
	}
	case 0x8D: {
		Opcode_8D_MOV_Direct_R5();
		break;
	}
	case 0x8E: {
		Opcode_8E_MOV_Direct_R6();
		break;
	}
	case 0x8F: {
		Opcode_8F_MOV_Direct_R7();
		break;
	}
	case 0x90: {
		Opcode_90_MOV_DPTR_Data();
		break;
	}
	case 0x91: {
		Opcode_91_ACALL_Addr11();
		break;
	}
	case 0x92: {
		Opcode_92_MOV_Bit_C();
		break;
	}
	case 0x93: {
		Opcode_93_MOVC_A_DPTR();
		break;
	}
		/*�����Ѿ�����PASS*/
	case 0x94: {
		Opcode_94_SUBB_A_Data();
		break;
	}
	case 0x95: {
		Opcode_95_SUBB_A_Direct();
		break;
	}
	case 0x96: {
		Opcode_96_SUBB_A_R0_1();
		break;
	}
	case 0x97: {
		Opcode_97_SUBB_A_R1_1();
		break;
	}
	case 0x98: {
		Opcode_98_SUBB_A_R0();
		break;
	}
	case 0x99: {
		Opcode_99_SUBB_A_R1();
		break;
	}
	case 0x9A: {
		Opcode_9A_SUBB_A_R2();
		break;
	}
	case 0x9B: {
		Opcode_9B_SUBB_A_R3();
		break;
	}
	case 0x9C: {
		Opcode_9C_SUBB_A_R4();
		break;
	}
	case 0x9D: {
		Opcode_9D_SUBB_A_R5();
		break;
	}
	case 0x9E: {
		Opcode_9E_SUBB_A_R6();
		break;
	}
	case 0x9F: {
		Opcode_9F_SUBB_A_R7();
		break;
	}
	case 0xA0: {
		Opcode_A0_ORL_C_Bit();
		break;
	}
	case 0xA1: {
		Opcode_A1_AJMP_Addr11();
		break;
	}
	case 0xA2: {
		Opcode_A2_MOV_C_Bit();
		break;
	}
	case 0xA3: {
		Opcode_A3_INC_DPTR();
		break;
	}
	case 0xA4: {
		Opcode_A4_MUL_AB();
		break;
	}
	case 0xA5: {
		Opcode_A5();
		break;
	}
	case 0xA6: {
		Opcode_A6_MOV_R0_1_Direct();
		break;
	}
	case 0xA7: {
		Opcode_A7_MOV_R1_1_Direct();
		break;
	}
	case 0xA8: {
		Opcode_A8_MOV_R0_Direct();
		break;
	}
	case 0xA9: {
		Opcode_A9_MOV_R1_Direct();
		break;
	}
	case 0xAA: {
		Opcode_AA_MOV_R2_Direct();
		break;
	}
	case 0xAB: {
		Opcode_AB_MOV_R3_Direct();
		break;
	}
	case 0xAC: {
		Opcode_AC_MOV_R4_Direct();
		break;
	}
	case 0xAD: {
		Opcode_AD_MOV_R5_Direct();
		break;
	}
	case 0xAE: {
		Opcode_AE_MOV_R6_Direct();
		break;
	}
	case 0xAF: {
		Opcode_AF_MOV_R7_Direct();
		break;
	}
	case 0xB0: {
		Opcode_B0_ANL_C_Bit_1();
		break;
	}
	case 0xB1: {
		Opcode_B1_ACALL_Addr11();
		break;
	}
	case 0xB2: {
		Opcode_B2_CPL_Bit();
		break;
	}
	case 0xB3: {
		Opcode_B3_CPL_C();
		break;
	}
	case 0xB4: {
		Opcode_B4_CJNE_A_Data_Rel();
		break;
	}
	case 0xB5: {
		Opcode_B5_CJNE_A_Direct_Rel();
		break;
	}
	case 0xB6: {
		Opcode_B6_CJNE_R0_1_Data_Rel();
		break;
	}
	case 0xB7: {
		Opcode_B7_CJNE_R1_1_Data_Rel();
		break;
	}
	case 0xB8: {
		Opcode_B8_CJNE_R0_Data_Rel();
		break;
	}
	case 0xB9: {
		Opcode_B9_CJNE_R1_Data_Rel();
		break;
	}
	case 0xBA: {
		Opcode_BA_CJNE_R2_Data_Rel();
		break;
	}
	case 0xBB: {
		Opcode_BB_CJNE_R3_Data_Rel();
		break;
	}
	case 0xBC: {
		Opcode_BC_CJNE_R4_Data_Rel();
		break;
	}
	case 0xBD: {
		Opcode_BD_CJNE_R5_Data_Rel();
		break;
	}
	case 0xBE: {
		Opcode_BE_CJNE_R6_Data_Rel();
		break;
	}
	case 0xBF: {
		Opcode_BF_CJNE_R7_Data_Rel();
		break;
	}
	case 0xC0: {
		Opcode_C0_PUSH_Direct();
		break;
	}
	case 0xC1: {
		Opcode_C1_AJMP_Addr11();
		break;
	}
	case 0xC2: {
		Opcode_C2_CLR_Bit();
		break;
	}
	case 0xC3: {
		Opcode_C3_CLR_C();
		break;
	}
	case 0xC4: {
		Opcode_C4_SWAP_A();
		break;
	}
	case 0xC5: {
		Opcode_C5_XCH_A_Direct();
		break;
	}
	case 0xC6: {
		Opcode_C6_XCH_A_R0_1();
		break;
	}
	case 0xC7: {
		Opcode_C7_XCH_A_R1_1();
		break;
	}
	case 0xC8: {
		Opcode_C8_XCH_A_R0();
		break;
	}
	case 0xC9: {
		Opcode_C9_XCH_A_R1();
		break;
	}
	case 0xCA: {
		Opcode_CA_XCH_A_R2();
		break;
	}
	case 0xCB: {
		Opcode_CB_XCH_A_R3();
		break;
	}
	case 0xCC: {
		Opcode_CC_XCH_A_R4();
		break;
	}
	case 0xCD: {
		Opcode_CD_XCH_A_R5();
		break;
	}
	case 0xCE: {
		Opcode_CE_XCH_A_R6();
		break;
	}
	case 0xCF: {
		Opcode_CF_XCH_A_R7();
		break;
	}
	case 0xD0: {
		Opcode_D0_POP_Direct();
		break;
	}
	case 0xD1: {
		Opcode_D1_ACALL_Addr11();
		break;
	}
	case 0xD2: {
		Opcode_D2_SETB_Bit();
		break;
	}
	case 0xD3: {
		Opcode_D3_SETB_C();
		break;
	}
	case 0xD4: {
		Opcode_D4_DA_A();
		break;
	}
	case 0xD5: {
		Opcode_D5_DJNZ_Direct_Rel();
		break;
	}
	case 0xD6: {
		Opcode_D6_XCHD_A_R0_1();
		break;
	}
	case 0xD7: {
		Opcode_D7_XCHD_A_R1_1();
		break;
	}
	case 0xD8: {
		Opcode_D8_DJNZ_R0_Rel();
		break;
	}
	case 0xD9: {
		Opcode_D9_DJNZ_R1_Rel();
		break;
	}
	case 0xDA: {
		Opcode_DA_DJNZ_R2_Rel();
		break;
	}
	case 0xDB: {
		Opcode_DB_DJNZ_R3_Rel();
		break;
	}
	case 0xDC: {
		Opcode_DC_DJNZ_R4_Rel();
		break;
	}
	case 0xDD: {
		Opcode_DD_DJNZ_R5_Rel();
		break;
	}
	case 0xDE: {
		Opcode_DE_DJNZ_R6_Rel();
		break;
	}
	case 0xDF: {
		Opcode_DF_DJNZ_R7_Rel();
		break;
	}
	case 0xE0: {
		Opcode_E0_MOVX_A_DPTR();
		break;
	}
	case 0xE1: {
		Opcode_E1_AJMP_Addr11();
		break;
	}
	case 0xE2: {
		Opcode_E2_MOVX_A_R0_1();
		break;
	}
	case 0xE3: {
		Opcode_E3_MOVX_A_R1_1();
		break;
	}
	case 0xE4: {
		Opcode_E4_CLR_A();
		break;
	}
	case 0xE5: {
		Opcode_E5_MOV_A_Direct();
		break;
	}
	case 0xE6: {
		Opcode_E6_MOV_A_R0_1();
		break;
	}
	case 0xE7: {
		Opcode_E7_MOV_A_R1_1();
		break;
	}
	case 0xE8: {
		Opcode_E8_MOV_A_R0();
		break;
	}
	case 0xE9: {
		Opcode_E9_MOV_A_R1();
		break;
	}
	case 0xEA: {
		Opcode_EA_MOV_A_R2();
		break;
	}
	case 0xEB: {
		Opcode_EB_MOV_A_R3();
		break;
	}
	case 0xEC: {
		Opcode_EC_MOV_A_R4();
		break;
	}
	case 0xED: {
		Opcode_ED_MOV_A_R5();
		break;
	}
	case 0xEE: {
		Opcode_EE_MOV_A_R6();
		break;
	}
	case 0xEF: {
		Opcode_EF_MOV_A_R7();
		break;
	}
	case 0xF0: {
		Opcode_F0_MOVX_DPTR_A();
		break;
	}
	case 0xF1: {
		Opcode_F1_ACALL_Addr11();
		break;
	}
	case 0xF2: {
		Opcode_F2_MOVX_R0_1_A();
		break;
	}
	case 0xF3: {
		Opcode_F3_MOVX_R1_1_A();
		break;
	}
	case 0xF4: {
		Opcode_F4_CPL_A();
		break;
	}
	case 0xF5: {
		Opcode_F5_MOV_Direct_A();
		break;
	}
	case 0xF6: {
		Opcode_F6_MOV_R0_1_A();
		break;
	}
	case 0xF7: {
		Opcode_F7_MOV_R1_1_A();
		break;
	}
	case 0xF8: {
		Opcode_F8_MOV_R0_A();
		break;
	}
	case 0xF9: {
		Opcode_F9_MOV_R1_A();
		break;
	}
	case 0xFA: {
		Opcode_FA_MOV_R2_A();
		break;
	}
	case 0xFB: {
		Opcode_FB_MOV_R3_A();
		break;
	}
	case 0xFC: {
		Opcode_FC_MOV_R4_A();
		break;
	}
	case 0xFD: {
		Opcode_FD_MOV_R5_A();
		break;
	}
	case 0xFE: {
		Opcode_FE_MOV_R6_A();
		break;
	}
	case 0xFF: {
		Opcode_FF_MOV_R7_A();
		break;
	}
	default:
		assert(0);
		break;
	}
//		if (tempa != Sys.a())
//				{
//			UpDataPFlag();
//		}
//	UpDataDebugInfo();
}
//static INT8U getAddr(INT8U addr) {
//	if (addr <= 0x7F) {
//		return (addr / 8) + 0x20;
//	} else {
//		return addr - ((addr - 0x80) % 8);
//	}
//	assert(0);
//	return addr;
//}

bool CVir8051::GetBitFlag(INT8U addr) {
	return (GetBitRamRef(addr) & (BIT0 << (addr % 8))) != 0;
}

//bool CVir8051::GetBitFlag(INT8U addr, INT8U n) const { // addr��Ԫ��ַ��n�Ƕ�Ӧ�ĵڼ�λ
//	INT8U temp;
//	Assert(addr <= 0xF0);
//	// memcpy(&temp,GetPointRamAddr((addr/8)+0x20),sizeof(temp));
//	// return (((BIT0<<(addr%8))& temp) != 0);
//	memcpy(&temp, GetPointRamAddr(addr), sizeof(temp));
//	return (((BIT0 << n) & temp) != 0);
//}
void CVir8051::SetBitFlag(INT8U addr) {

//	INT8U tempaddr = getAddr(addr);
	GetBitRamRef(addr) |= (BIT0 << (addr % 8));
	if (addr >= 0xe0 && addr <= 0xe7) { //this opcode A
		Updata_A_P_Flag();
	}
}

void CVir8051::ClrBitFlag(INT8U addr) {
//	Assert(addr <= 0xF0);
//	INT8U temp;
//	//void  *p = GetPointRamAddr((addr/8)+0x20);
//	void *p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
//	//temp &=(~(BIT0<<(addr%8)));
//	temp &= (~(BIT0 << n));
//	memcpy(p, &temp, sizeof(temp));
//	INT8U tempaddr = getAddr(addr);

	GetBitRamRef(addr) &= (~(BIT0 << (addr % 8)));
	if (addr >= 0xe0 && addr <= 0xe7) //this opcode A
			{
		Updata_A_P_Flag();
	}
//
//
//INT8U tempaddr = 0;
//if (addr <= 0x7F) {
//	tempaddr = (addr / 8) + 0x20;
//	m_ChipRam[tempaddr] &= (~(BIT0 << (addr % 8)));
//} else {
//	addr -= 0x80;
//	tempaddr = (addr / 8);
//	m_ChipSfr[tempaddr] &= (~(BIT0 << (addr % 8)));
//}
}

INT8U CVir8051::GetOpcode(void) const {
	return m_ExeFile[Sys.PC];
}

INT8U& CVir8051::GetRamRef(INT8U addr) {

//	static const INT8U sfrarry[] = { a_addr, b_addr, dptrl, dptrh, psw_addr, sp_addr };
//	if (find(sfrarry, sfrarry + sizeof(sfrarry), addr) != sfrarry + sizeof(sfrarry)) {
	if (addr > 0x7F) {
		return m_ChipSfr[addr];
	} else {
		return m_ChipRam[addr];
	}
}

INT16U CVir8051::GetDebugOpcode(void) const {
	INT16U temp = 0;
	memcpy(&temp, &m_ExeFile[Sys.PC], 2);
	return temp;
}

bool CVir8051::GetDebugPC(INT16U pc) const {
	return (pc == Sys.PC);
}

void CVir8051::GetOpcodeData(void * const p, INT8U len) const {  //��ȡ��λ�ֽ�
	memcpy(p, &m_ExeFile[Sys.PC + 1], len);
}
void CVir8051::AJMP(INT8U opCode, INT8U data) {
	INT16U tmppc = Sys.PC + 2;
// tmppc  |= ((((((INT16U)opCode)<<5)&(0x7)))|((INT16U)data));
	tmppc &= 0xF800;
	tmppc |= ((((((INT16U) opCode) >> 5) & 0x7) << 8) | ((INT16U) data));

	Sys.PC = tmppc;
}
void CVir8051::ACALL(INT8U opCode) {
	INT8U data = 0;
	GetOpcodeData(&data, sizeof(data));

	Sys.PC += 2;
	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC & 0x00ff);
//	memcpy(p, &temp, sizeof(temp));
//	m_ChipRam[Sys.sp()]=(INT8U) (Sys.PC & 0x00ff);
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC & 0x00ff));

	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC >> 8);
//	memcpy(p, &temp, sizeof(temp));
//	SetRamData(Sys.sp(), (INT8U) (INT8U) (Sys.PC >> 8));
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC >> 8));
//	temp = (INT8U)(data>>8);
//	data = (data<<8)|temp;
	Sys.PC = ((Sys.PC & 0xF800) | (((((INT16U) opCode) >> 5) << 8) | (INT16U) data));
}

void CVir8051::UpDataDebugInfo(void) {
	d_Rges.R0 = Rges.R0();
	d_Rges.R1 = Rges.R1();
	d_Rges.R2 = Rges.R2();
	d_Rges.R3 = Rges.R3();
	d_Rges.R4 = Rges.R4();
	d_Rges.R5 = Rges.R5();
	d_Rges.R6 = Rges.R6();
	d_Rges.R7 = Rges.R7();

	d_Sys.a = Sys.a();
	d_Sys.b = Sys.b();
	d_Sys.sp = Sys.sp();
	d_Sys.dptr = Sys.dptr();
	d_Sys.psw = Sys.psw();
	d_Sys.PC = Sys.PC;
}

void * CVir8051::GetExRamAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ExRam));
	return (void *) &m_ExRam[addr];
}
INT8U CVir8051::GetExRam(INT16U addr) const {
	Assert(addr < sizeof(m_ExRam));
	return m_ExRam[addr];
}
INT8U CVir8051::SetExRam(INT16U addr, INT8U data) {
	Assert(addr < sizeof(m_ExRam));
	return m_ExRam[addr] = data;
}
void * CVir8051::GetPointRamAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ChipRam));
	return (void *) &m_ChipRam[addr];
}

INT8U CVir8051::GetRamDataAt(INT8U addr) {
	return m_ChipRam[addr];
}
INT8U CVir8051::SetRamDataAt(INT8U addr, INT8U data) {
	return m_ChipRam[addr] = data;
}
INT8U CVir8051::GetRamData(INT8U addr) {
	return GetRamRef(addr);
}
INT8U CVir8051::SetRamData(INT8U addr, INT8U data) {
	GetRamRef(addr) = data;
	return data;
}

void* CVir8051::GetPointFileAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ExeFile));
	return (void*) &m_ExeFile[addr];
}

void CVir8051::Opcode_00_NOP(void) {
	Sys.PC++;
}

void CVir8051::Opcode_01_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x01, temp);
}

void CVir8051::Opcode_21_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x21, temp);
}
void CVir8051::Opcode_41_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x41, temp);
}
void CVir8051::Opcode_61_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x61, temp);
}
void CVir8051::Opcode_81_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x81, temp);
}
void CVir8051::Opcode_A1_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0xA1, temp);
}
void CVir8051::Opcode_C1_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0xC1, temp);
}
void CVir8051::Opcode_E1_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0xE1, temp);
}
void CVir8051::Opcode_02_LJMP_Addr16(void) {
	INT8U temp[2];
	INT16U data;
	GetOpcodeData(temp, 2);
	data = (((INT16U) (temp[0])) << 8);
	Sys.PC = data | temp[1];
}

void CVir8051::Opcode_03_RR_A(void) {
	INT8U temp = (Sys.a() & 0x1);
	Sys.a = Sys.a() >> 1;
	Sys.a = Sys.a() | (temp << 7);
//	Sys.a =Sys.a() | (temp << 7);

	Sys.PC++;
}
void CVir8051::Opcode_04_INC_A(void) {
	Sys.a = Sys.a() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_05_INC_Direct(void) {
	INT8U temp = 0;
	INT8U addr = 0;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	++temp;
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC = Sys.PC + 2;
}

void CVir8051::Opcode_06_INC_R0_1(void) {
	INT8U data = 0;
	INT8U addr = Rges.R0();
	data = GetRamDataAt(addr);
	++data;
	SetRamDataAt(addr, data);
//	++m_ChipRam[Rges.R0()];
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
//	temp++;
//	memcpy(p, &temp, sizeof(temp));
	Sys.PC++;
}
void CVir8051::Opcode_07_INC_R1_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
//	temp++;
//	memcpy(p, &temp, sizeof(temp));

	INT8U data = 0;
	INT8U addr = Rges.R1();
	data = GetRamDataAt(addr);
	++data;
	SetRamDataAt(addr, data);
//	++m_ChipRam[Rges.R1()];
	Sys.PC++;
}

void CVir8051::Opcode_08_INC_R0(void) {
	Rges.R0 = Rges.R0() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_09_INC_R1(void) {
	Rges.R1 = Rges.R1() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0A_INC_R2(void) {
	Rges.R2 = Rges.R2() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0B_INC_R3(void) {
	Rges.R3 = Rges.R3() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0C_INC_R4(void) {
	Rges.R4 = Rges.R4() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0D_INC_R5(void) {
	Rges.R5 = Rges.R5() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0E_INC_R6(void) {
	Rges.R6 = Rges.R6() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0F_INC_R7(void) {
	Rges.R7 = Rges.R7() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_10_JBC_Bit_Rel(void) {
	INT8U temp[2];
	char tem2;

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
//	Assert(temp[0]<=0xF7);

//	if (temp[0] <= 0x7F) {
	if (GetBitFlag((temp[0]))) {
		ClrBitFlag(temp[0]);
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
	}
//	} else {
//		if (GetBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8)) {
//			ClrBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8);
//			Sys.PC = GetTargPC(temp[1]);
//			;
//		}
//	}
}

void CVir8051::Opcode_11_ACALL_Addr11(void) {
	ACALL(0x11);
}
void CVir8051::Opcode_31_ACALL_Addr11(void) {
	ACALL(0x31);
}
void CVir8051::Opcode_51_ACALL_Addr11(void) {
	ACALL(0x51);
}
void CVir8051::Opcode_71_ACALL_Addr11(void) {
	ACALL(0x71);
}
void CVir8051::Opcode_91_ACALL_Addr11(void) {
	ACALL(0x91);
}
void CVir8051::Opcode_B1_ACALL_Addr11(void) {
	ACALL(0xB1);
}
void CVir8051::Opcode_D1_ACALL_Addr11(void) {
	ACALL(0xD1);
}
void CVir8051::Opcode_F1_ACALL_Addr11(void) {
	ACALL(0xF1);
}

INT16U CVir8051::GetLcallAddr(void) {
	INT16U addr = 0;
	GetOpcodeData((INT8U*) &addr, 2);
	Assert(GetOpcode() == 0x12);
	return (addr >> 8) | (addr << 8);

}
//
//bool CVir8051::SubstitutionLcall(char *RetData,enum FUN_TYPE type,INT16U Retlen)
//{
//    static const INT16U map[][2] =
//    {
//        {RET_VOID__VOID,0},
//        {RET_VOID__PARA_CHAR,0},
//        {RET_VOID__PARA_CHAR_CHAR,0},
//        {RET_VOID__PARA_CHAR_CHAR_P,0},
//
//        {RET_CHAR__VOID,1},
//        {RET_CHAR__VOID,1},
//        {RET_VOID__PARA_CHAR_CHAR,1},
//        {RET_VOID__PARA_CHAR_CHAR_P,1},
//    };
//    INT16U i = sizeof(map)/(sizeof(INT16U)*2);
//    INT16U temp =0xFFFF;
//
//    while(i--)
//    {
//        if(map[i][0] == type)
//            {
//               temp = map[i][1];
//               break;
//            }
//    }
//
//
//    Sys.PC += 3;
//
//    if(temp ==0xFFFF)
//    {
//        Assert(0);
//    }
//      switch (temp)
//        {
//
//            case 0:
//            {
//                 return true;
//            }
//           case 1:
//            {
//               Rges.R7 = RetData[0];//���뷵������
//              return true;
//            }
//
//        }
//    return false;
//}

//
//bool CVir8051::GetParameter(char *rxbuffer,enum FUN_TYPE type,INT16U rxlen)
//    {
//    INT32U Addr;
//    void *p_tem;
//    switch (type)
//        {
//        case RET_VOID__VOID:
//                {
//                     return true;
//                 }
//          case RET_CHAR__PARA_CHAR_CHAR:
//          case RET_VOID__PARA_CHAR_CHAR:
//            {
//                Assert(rxlen >=2);
//                rxbuffer[0]= Rges.R7();
//                rxbuffer[1]= Rges.R5();
//                 return true;
//             }
//
//           case RET_CHAR__VOID:
//           case RET_VOID__PARA_CHAR:
//            {
//                Assert(rxlen >= 1);
//                 rxbuffer[0] = Rges.R7();
//                 return true;
//            }
//
//           case  RET_CHAR__PARA_CHAR_CHAR_P:
//           case RET_VOID__PARA_CHAR_CHAR_P:
//            {
//                rxbuffer[0]= Rges.R7();
//                rxbuffer[1]= Rges.R5();
//                rxbuffer[2]= Rges.R1();
//                rxbuffer[3]= Rges.R2();
//                rxbuffer[4]= Rges.R3();
//                Addr = 0 ;
//                memcpy(&Addr,&rxbuffer[2],3);
//                p_tem = Chang8051pToSelfp(Addr);
//                memcpy(&rxbuffer[2],&p_tem,sizeof(p_tem));
//                return true;
//            }
//        }
//    return false;
//    }

//void CVir8051::SubstitutionLcall(INT16U ret)
//    {
//       Sys.PC+=3;
//       Rges.R6 =  ((INT8U)(ret>>8));
//       Rges.R7 = ((INT8U)(ret));
//    }

void CVir8051::Opcode_12_LCALL_Addr16(void) {
//	INT8U temp = 0;
	INT16U addr = 0;
//	void *p = NULL;

	GetOpcodeData((INT8U*) &addr, 2);
	Sys.PC += 3;
	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC & 0x00ff);
//	memcpy(p, &temp, sizeof(temp));
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC & 0x00ff));
	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC >> 8);
//	memcpy(p, &temp, sizeof(temp));
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC >> 8));
// GetOpcodeData(&Sys.PC,2);
	Sys.PC = (addr >> 8) | (addr << 8);
}
void CVir8051::Opcode_13_RRC_A(void) {
	INT8U temp = Sys.a() & BIT0;

	Sys.a = ((Sys.a() >> 1) | (Sys.psw().cy << 7));

	Sys.psw().cy = (temp == 0) ? 0 : 1;
	Sys.PC++;
}
void CVir8051::Opcode_14_DEC_A(void) {
	Sys.a = Sys.a() - 1;
	Sys.PC++;
}
void CVir8051::Opcode_15_DEC_Direct(void) {
	INT8U temp = 0;
	INT8U addr = 0;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);

//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	temp--;
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC = Sys.PC + 2;
}
void CVir8051::Opcode_16_DEC_R0_1(void) {
	INT8U temp = 0;
	INT8U addr = Rges.R0();
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(addr);
	temp--;
	SetRamDataAt(addr, temp);
	Sys.PC++;
}
void CVir8051::Opcode_17_DEC_R1_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
//	temp--;
//	memcpy(p, &temp, sizeof(temp));
//	Sys.PC++;

	INT8U temp = 0;
	INT8U addr = Rges.R1();
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(addr);
	temp--;
	SetRamDataAt(addr, temp);
	Sys.PC++;

}
void CVir8051::Opcode_18_DEC_R0(void) {
//	Rges.R0--;
	Rges.R0 = Rges.R0() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_19_DEC_R1(void) {
	Rges.R1 = Rges.R1() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1A_DEC_R2(void) {
	Rges.R2 = Rges.R2() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1B_DEC_R3(void) {
	Rges.R3 = Rges.R3() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1C_DEC_R4(void) {
	Rges.R4 = Rges.R4() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1D_DEC_R5(void) {
	Rges.R5 = Rges.R5() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1E_DEC_R6(void) {
	Rges.R6 = Rges.R6() - 1;
	;
	Sys.PC++;
}
void CVir8051::Opcode_1F_DEC_R7(void) {
	Rges.R7 = Rges.R7() - 1;
	;
	Sys.PC++;
}
void CVir8051::Opcode_20_JB_Bit_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (GetBitFlag((temp[0]))) {
		char tem2;
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp[1]);
	}

//	Assert(temp[0]<0xF7);
//	if (temp[0] <= 0x7F) {
//		if (GetBitFlag((temp[0] / 8) + 0x20, temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	} else {
//		if (GetBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	}
}

void CVir8051::Opcode_22_RET(void) {
	INT8U temp = 0;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Sys.sp());
	Sys.PC = (Sys.PC & 0x00FF) | (((INT16U) temp) << 8);
// memcpy((INT8U*)(&Sys.PC)+1,&temp,1);
	Sys.sp = Sys.sp() - 1;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Sys.sp());
	Sys.PC = (Sys.PC & 0xFF00) | temp;
//memcpy((INT8U*)(&Sys.PC),&temp,1);
	Sys.sp = Sys.sp() - 1;
}

void CVir8051::Opcode_23_RL_A(void) {
	INT8U temp = Sys.a() & 0x80;
	INT8U tem2 = Sys.a();
	Sys.a = (INT8U) ((tem2 << 1) | (temp >> 7));
	Sys.PC++;
}

void CVir8051::MD_ADDC(INT8U data) {
	INT8U flagAC = Sys.psw().cy;

	if (flagAC > 1) {
		assert(0);
		flagAC = 1;
	}

	INT16U tep = (INT16U) Sys.a() + (INT16U) data + flagAC;

	INT16U tep2 = ((INT16U) (Sys.a() & 0x7F) + (INT16U) (data & 0x7F)) + flagAC;

	if ((Sys.a() & 0x0f) + (data & 0x0f) + flagAC > 0x0F) {
		Sys.psw().ac = 1;
	} else {
		Sys.psw().ac = 0;

	}
	if (tep > 0xFF) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;

	}

	flagAC = 0;

	if (tep > 0xFF) {
		flagAC++;
	}

	if (tep2 > 0x7F) {
		flagAC++;
	}

	Sys.psw().ov = flagAC == 1 ? 1 : 0;

	Sys.a = (INT8U) tep;

}
void CVir8051::MD_SUBB(INT8U data) {
	INT16U tepa = Sys.a();

	INT8U bcy = Sys.psw().cy;
//	data += Sys.psw().cy;

	if ((tepa & 0x0f) < (data & 0x0f) + Sys.psw().cy) {
		Sys.psw().ac = 1;
	} else {

		Sys.psw().ac = 0;

	}

	if (tepa < data + bcy) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;
	}

//��������λ
	INT8U flagac = 0;
	if (tepa < data + bcy) {
		flagac++;
	}

	if ((tepa & 0x7F) < (data & 0x7F) + bcy) {

		flagac++;

	}

	Sys.psw().ov = flagac == 1 ? 1 : 0;
	Sys.a = (INT8U) (tepa - data - bcy);

}

void CVir8051::MD_ADD(INT8U data) {
	INT16U tep = (INT16U) Sys.a() + (INT16U) data;

	INT16U tep2 = ((INT16U) (Sys.a() & 0x7F) + (INT16U) (data & 0x7F));

	if ((Sys.a() & 0x0f) + (data & 0x0f) > 0x0F) {
		Sys.psw().ac = 1;
	} else {
		Sys.psw().ac = 0;
	}

	if (tep > 0xFF) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;
	}

	INT8U flagAC = 0;

	if (tep > 0xFF) {
		flagAC++;
	}

	if (tep2 > 0x7F) {
		flagAC++;
	}

	Sys.psw().ov = flagAC == 1 ? 1 : 0;

	Sys.a = (INT8U) tep;

}

void CVir8051::Opcode_24_ADD_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	MD_ADD(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_25_ADD_A_Direct(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));

	temp = GetRamData(addr);
	MD_ADD(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_26_ADD_A_R0_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
	temp = GetRamDataAt(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	MD_ADD(temp);
//Sys.a +=temp;
	Sys.PC++;
}
void CVir8051::Opcode_27_ADD_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	MD_ADD(temp);
//Sys.a +=temp;
	Sys.PC++;
}
void CVir8051::Opcode_28_ADD_A_R0(void) {
	MD_ADD(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_29_ADD_A_R1(void) {
	MD_ADD(Rges.R1());
	Sys.PC++;
}
void CVir8051::Opcode_2A_ADD_A_R2(void) {
	MD_ADD(Rges.R2());
	Sys.PC++;
}
void CVir8051::Opcode_2B_ADD_A_R3(void) {
	MD_ADD(Rges.R3());
	Sys.PC++;
}
void CVir8051::Opcode_2C_ADD_A_R4(void) {
	MD_ADD(Rges.R4());
	Sys.PC++;
}
void CVir8051::Opcode_2D_ADD_A_R5(void) {
	MD_ADD(Rges.R5());
	Sys.PC++;
}
void CVir8051::Opcode_2E_ADD_A_R6(void) {
	MD_ADD(Rges.R6());
	Sys.PC++;
}
void CVir8051::Opcode_2F_ADD_A_R7(void) {
	MD_ADD(Rges.R7());
	Sys.PC++;
}
void CVir8051::Opcode_30_JNB_Bit_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
//	Assert(temp[0]<0xF7);

	Sys.PC += 3;
	if (!GetBitFlag((temp[0]))) {
		char tem2;
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp[1]);
	}

//	if (temp[0] <= 0x7F) {
//		if (!GetBitFlag((temp[0] / 8) + 0x20, temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	} else {
//		if (!GetBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	}
}

void CVir8051::Opcode_32_RETI(void) {
	INT8U temp = 0;
//	void *p = NULL;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(Sys.sp());
	Sys.PC = (Sys.PC & 0x00FF) | (((INT16U) temp) << 8);
// memcpy((INT8U*)(&Sys.PC)+1,&temp,1);
	Sys.sp = Sys.sp() - 1;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(Sys.sp());
	Sys.PC = (Sys.PC & 0xFF00) | temp;
//memcpy((INT8U*)(&Sys.PC),&temp,1);
	Sys.sp = Sys.sp() - 1;
	Sys.PC++;
}
void CVir8051::Opcode_33_RLC_A(void) {
	INT8U temp = Sys.a() & 0x80;
	Sys.a = ((Sys.a() << 1) | Sys.psw().cy);
	Sys.psw().cy = temp == 0 ? 0 : 1;

	Sys.PC++;
}
void CVir8051::Opcode_34_ADDC_A_Data(void) {
	INT8U data;
	GetOpcodeData(&data, 1);
	MD_ADDC(data);   //����

	Sys.PC += 2;
}
void CVir8051::Opcode_35_ADDC_A_Direct(void) {
	INT8U data;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	data = GetRamData(addr);
	MD_ADDC(data);

	Sys.PC += 2;
}
void CVir8051::Opcode_36_ADDC_A_R0_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R0());
	MD_ADDC(temp);

	Sys.PC++;
}
void CVir8051::Opcode_37_ADDC_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());

// GetOpcodeData(&temp,1);
	MD_ADDC(temp);

	Sys.PC++;
}
void CVir8051::Opcode_38_ADDC_A_R0(void) {

//GetOpcodeData(&temp,1);
	MD_ADDC(Rges.R0());

	Sys.PC++;
}
void CVir8051::Opcode_39_ADDC_A_R1(void) {
	MD_ADDC(Rges.R1());

	Sys.PC++;
}
void CVir8051::Opcode_3A_ADDC_A_R2(void) {
	MD_ADDC(Rges.R2());

	Sys.PC++;
}
void CVir8051::Opcode_3B_ADDC_A_R3(void) {
	MD_ADDC(Rges.R3());

	Sys.PC++;
}
void CVir8051::Opcode_3C_ADDC_A_R4(void) {
	MD_ADDC(Rges.R4());

	Sys.PC++;
}
void CVir8051::Opcode_3D_ADDC_A_R5(void) {
	MD_ADDC(Rges.R5());

	Sys.PC++;
}
void CVir8051::Opcode_3E_ADDC_A_R6(void) {
	MD_ADDC(Rges.R6());

	Sys.PC++;
}
void CVir8051::Opcode_3F_ADDC_A_R7(void) {
	MD_ADDC(Rges.R7());

	Sys.PC++;
}
void CVir8051::Opcode_40_JC_Rel(void) {
	char tem2;
	GetOpcodeData(&tem2, 1);
	Sys.PC += 2;
	if (Sys.psw().cy) {
		memcpy(&tem2, &tem2, sizeof(tem2));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp);
	}
}

void CVir8051::Opcode_42_ORL_Direct_A(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	temp |= Sys.a();
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_43_ORL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2); //direct ~{JG~}1~{8vWV=Z~}
//	p = GetPointRamAddr(temp[0]);
//	memcpy(&data, p, 1);
	data = GetRamData(temp[0]);
	data |= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVir8051::Updata_A_P_Flag(void) {
	INT8U a = Sys.a();
	a ^= a >> 4;
	a ^= a >> 2;
	a ^= a >> 1;
	Sys.psw().p = (a & 1);

}
void CVir8051::Opcode_44_ORL_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = Sys.a() | temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_45_ORL_A_Direct(void) {
	INT8U data;
	INT8U addr;
	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
	Sys.a = Sys.a() | data;
	Sys.PC += 2;
}
void CVir8051::Opcode_46_ORL_A_R0_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = Sys.a() | GetRamDataAt(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_47_ORL_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	Sys.a = Sys.a() | temp;
	Sys.PC++;
}
void CVir8051::Opcode_48_ORL_A_R0(void) {
	Sys.a = Sys.a() | Rges.R0();
	Sys.PC++;
}
void CVir8051::Opcode_49_ORL_A_R1(void) {
	Sys.a = Sys.a() | Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_4A_ORL_A_R2(void) {
	Sys.a = Sys.a() | Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_4B_ORL_A_R3(void) {
	Sys.a = Sys.a() | Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_4C_ORL_A_R4(void) {
	Sys.a = Sys.a() | Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_4D_ORL_A_R5(void) {
	Sys.a = Sys.a() | Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_4E_ORL_A_R6(void) {
	Sys.a = Sys.a() | Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_4F_ORL_A_R7(void) {
	Sys.a = Sys.a() | Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_50_JNC_Rel(void) {
	char tem2;
	GetOpcodeData(&tem2, 1);
	Sys.PC += 2;
	if (Sys.psw().cy == 0) {
		Sys.PC += tem2;
	}
}
void CVir8051::Opcode_52_ANL_Direct_A(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
	temp = GetRamData(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp &= Sys.a();
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);

	Sys.PC += 2;
}
void CVir8051::Opcode_53_ANL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2);
//	p = GetPointRamAddr(temp[0]);
//	memcpy(&data, p, 1);
	data = GetRamData(temp[0]);
	data &= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVir8051::Opcode_54_ANL_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = Sys.a() & temp;
	Sys.PC += 2;
}

void CVir8051::Opcode_55_ANL_A_Direct(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, 1);
	temp = GetRamData(addr);
	Sys.a = Sys.a() & temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_56_ANL_A_R0_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = Sys.a() & GetRamDataAt(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_57_ANL_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	Sys.a = Sys.a() & temp;
	Sys.PC++;
}
void CVir8051::Opcode_58_ANL_A_R0(void) {
	Sys.a = Sys.a() & Rges.R0();
	Sys.PC++;
}
void CVir8051::Opcode_59_ANL_A_R1(void) {
	Sys.a = Sys.a() & Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_5A_ANL_A_R2(void) {
	Sys.a = Sys.a() & Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_5B_ANL_A_R3(void) {
	Sys.a = Sys.a() & Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_5C_ANL_A_R4(void) {
	Sys.a = Sys.a() & Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_5D_ANL_A_R5(void) {
	Sys.a = Sys.a() & Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_5E_ANL_A_R6(void) {
	Sys.a = Sys.a() & Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_5F_ANL_A_R7(void) {
	Sys.a = Sys.a() & Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_60_JZ_Rel(void) {
	char temp;
	GetOpcodeData(&temp, 1);
	Sys.PC += 2;
	if (Sys.a() == 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_62_XRL_Direct_A(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	temp ^= Sys.a();
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC += 2;
}

void CVir8051::Opcode_63_XRL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2);
//	p = GetPointRamAddr(temp[0]);
//	memcpy(&data, p, 1);
	data = GetRamData(temp[0]);
	data ^= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVir8051::Opcode_64_XRL_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = Sys.a() ^ temp;
	Sys.PC += 2;
}

void CVir8051::Opcode_65_XRL_A_Direct(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, 1);
	temp = GetRamData(addr);
	Sys.a = Sys.a() ^ temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_66_XRL_A_R0_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R0());
	Sys.a = Sys.a() ^ temp;
	Sys.PC++;
}
void CVir8051::Opcode_67_XRL_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	Sys.a = Sys.a() ^ temp;
	Sys.PC++;
}
void CVir8051::Opcode_68_XRL_A_R0(void) {
	Sys.a = Sys.a() ^ Rges.R0();
	Sys.PC++;
}
void CVir8051::Opcode_69_XRL_A_R1(void) {
	Sys.a = Sys.a() ^ Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_6A_XRL_A_R2(void) {
	Sys.a = Sys.a() ^ Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_6B_XRL_A_R3(void) {
	Sys.a = Sys.a() ^ Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_6C_XRL_A_R4(void) {
	Sys.a = Sys.a() ^ Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_6D_XRL_A_R5(void) {
	Sys.a = Sys.a() ^ Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_6E_XRL_A_R6(void) {
	Sys.a = Sys.a() ^ Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_6F_XRL_A_R7(void) {
	Sys.a = Sys.a() ^ Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_70_JNZ_Rel(void) {
	char temp;
	GetOpcodeData(&temp, 1);
	Sys.PC += 2;
	if (Sys.a() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_72_ORL_C_Direct(void) {
	INT8U addr, data;
	GetOpcodeData(&addr, 1);

	data = GetBitFlag(addr);
//	Assert(temp<0xF7);
//	if (temp <= 0x7F) {
//		data = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		data = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
//	temp = 0;
//	if (Sys.psw().cy) {
//		temp++;
//	}
//	if (data) {
//		temp++;
//	}
	Sys.psw().cy = Sys.psw().cy | data;

	Sys.PC += 2;
}
void CVir8051::Opcode_73_JMP_A_DPTR(void) {
	Sys.PC++;
	Sys.PC = (INT16U) Sys.a() + Sys.dptr();
}
void CVir8051::Opcode_74_MOV_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = temp;

	Sys.PC += 2;
}
void CVir8051::Opcode_75_MOV_Direct_Data(void) {
	INT8U temp[2] = { 0 };
//	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2);
//	p = GetPointRamAddr(temp[0]);

//	data = temp[1];
	SetRamData(temp[0], temp[1]);
//	memcpy(p, &data, 1);
	Sys.PC += 3;
}
void CVir8051::Opcode_76_MOV_R0_1_Data(void) {
//	INT8U temp;
//	INT8U data;
//	void *p = GetPointRamAddr(Rges.R0());
	GetOpcodeData(&m_ChipRam[Rges.R0()], 1);
//	data = temp;
//	memcpy(p, &data, sizeof(data));
	Sys.PC += 2;
}
void CVir8051::Opcode_77_MOV_R1_1_Data(void) {
//	INT8U temp;
//	INT8U data;
//	void *p = GetPointRamAddr(Rges.R1());
//	GetOpcodeData(&temp, 1);
//	data = temp;
//	memcpy(p, &data, sizeof(data));
	GetOpcodeData(&m_ChipRam[Rges.R1()], 1);
	Sys.PC += 2;
}
void CVir8051::Opcode_78_MOV_R0_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R0 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_79_MOV_R1_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R1 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7A_MOV_R2_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R2 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7B_MOV_R3_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R3 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7C_MOV_R4_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R4 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7D_MOV_R5_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R5 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7E_MOV_R6_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R6 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7F_MOV_R7_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R7 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_80_SJMP_Rel(void) {
	char temp;
	GetOpcodeData(&temp, 1);
//	printf("temp hex %d \r\n",temp);
	Sys.PC += 2;
//	Sys.PC = GetTargPC(temp);
	Sys.PC += temp;
}
void CVir8051::Opcode_82_ANL_C_Bit(void) {
//	INT8U temp;
//	GetOpcodeData(&temp, 1);
	Sys.psw().cy = Sys.psw().cy & GetBitFlag(m_ExeFile[Sys.PC + 1]);
	Sys.PC += 2;
}
void CVir8051::Opcode_83_MOVC_A_PC(void) {
//	INT8U temp;
	INT16U addr;
//	void*p = NULL;
	++Sys.PC;
	addr = (INT16U) (Sys.PC + Sys.a());
//	p = GetPointFileAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = (Sys.PC > CVir8051::MAX_ROM - Sys.a()) ? (0x00) : (m_ExeFile[addr]);

}
void CVir8051::Opcode_84_DIV_AB(void) {
	INT8U data1, data2;
	data1 = Sys.a();
	data2 = Sys.b();
	if (data2 == 0) {
		Sys.psw().ov = 1;
		goto Ret;
	}
	Sys.a = data1 / data2;
	Sys.b = data1 % data2;
	Sys.psw().ov = 0;
	Ret: Sys.psw().cy = 0;
	Sys.PC++;
}
void CVir8051::Opcode_85_MOV_Direct_Direct(void) {
	INT8U temp[2];
//	INT8U data[2];
//	void *p0 = NULL;
//	void *p1 = NULL;
	INT8U tem;
	GetOpcodeData(&temp[0], 2);
//	p0 = GetPointRamAddr(temp[0]);
	tem = GetRamData(temp[0]);
//	memcpy(&data[0], p0, 1);
//	p1 = GetPointRamAddr(temp[1]);
//	memcpy(p1, &data[0], 1);
	SetRamData(temp[1], tem);
	Sys.PC += 3;
}
void CVir8051::Opcode_86_MOV_Direct_R0_1(void) {
	INT8U addr;
	INT8U data;
//	void*p0 = NULL;
//	void *p1 = NULL;
	GetOpcodeData(&addr, 1);
//	p0 = GetPointRamAddr(temp);
//	p1 = GetPointRamAddr(Rges.R0());
	data = GetRamDataAt(Rges.R0());
//	memcpy(&data, p1, 1);
//	memcpy(p0, &data, 1);
	SetRamData(addr, data);
	Sys.PC += 2;
}
void CVir8051::Opcode_87_MOV_Direct_R1_1(void) {
	INT8U addr;
	INT8U data;
//	void*p0 = NULL;
//	void *p1 = NULL;
	GetOpcodeData(&addr, 1);
//	p0 = GetPointRamAddr(temp);
//	p1 = GetPointRamAddr(Rges.R1());
	data = GetRamDataAt(Rges.R1());
//	memcpy(&data, p1, 1);
//	memcpy(p0, &data, 1);
	SetRamData(addr, data);
	Sys.PC += 2;
}
void CVir8051::Opcode_88_MOV_Direct_R0(void) {
	INT8U addr;
//	void*p = NULL;
	GetOpcodeData(&addr, 1);
	SetRamData(addr, Rges.R0());

//	temp = Rges.R0();
//	memcpy(p, &temp, 1);
	Sys.PC += 2;
}
void CVir8051::Opcode_89_MOV_Direct_R1(void) {
	INT8U addr;
//	void*p = NULL;
	GetOpcodeData(&addr, 1);
	SetRamData(addr, Rges.R1());
//	p = GetPointRamAddr(temp);
//	temp = Rges.R1();
//	memcpy(p, &temp, 1);
	Sys.PC += 2;
}
void CVir8051::Opcode_8A_MOV_Direct_R2(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R2());
	Sys.PC += 2;
}
void CVir8051::Opcode_8B_MOV_Direct_R3(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R3());
	Sys.PC += 2;
}
void CVir8051::Opcode_8C_MOV_Direct_R4(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R4());
	Sys.PC += 2;
}
void CVir8051::Opcode_8D_MOV_Direct_R5(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R5());
	Sys.PC += 2;
}
void CVir8051::Opcode_8E_MOV_Direct_R6(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R6());
	Sys.PC += 2;
}
void CVir8051::Opcode_8F_MOV_Direct_R7(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R7());
	Sys.PC += 2;
}
void CVir8051::Opcode_90_MOV_DPTR_Data(void) {
	INT16U temp;
	GetOpcodeData(&temp, 2);
	temp = (temp >> 8) | (temp << 8);
	Sys.dptr = temp;
	Sys.PC += 3;
}
void CVir8051::Opcode_92_MOV_Bit_C(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
//	Assert(temp<=0xF7);
	if (Sys.psw().cy) {
//		if (temp <= 0x7F) {
//			SetBitFlag((temp / 8) + 0x20, temp % 8);
//		} else {
//			SetBitFlag(ChangeBitAddr(temp), temp % 8);
//		}
		SetBitFlag(temp);
	} else {
//		if (temp <= 0x7F) {
//			ClrBitFlag((temp / 8) + 0x20, temp % 8);
//		} else {
//			ClrBitFlag(ChangeBitAddr(temp), temp % 8);
//		}
		ClrBitFlag(temp);
	}
	Sys.PC += 2;
}
void CVir8051::Opcode_93_MOVC_A_DPTR(void) {
	INT8U temp;
	void *p = NULL;
	p = GetPointFileAddr((INT16U) (Sys.a() + Sys.dptr()));
	memcpy(&temp, p, sizeof(temp));
	Sys.a = (Sys.dptr() > CVir8051::MAX_ROM - Sys.a()) ? (0x00) : (temp);
	Sys.PC++;
}

void CVir8051::Opcode_94_SUBB_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	MD_SUBB(temp);

	Sys.PC += 2;
}
void CVir8051::Opcode_95_SUBB_A_Direct(void) {
	INT8U addr;
	INT8U data;
//	void *p = NULL;

	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&data, p, 1);
	data = GetRamData(addr);
	MD_SUBB(data);
	Sys.PC += 2;
}
void CVir8051::Opcode_96_SUBB_A_R0_1(void) {
//	INT8U temp;
//	void *p = NULL;
//	p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	MD_SUBB(GetRamDataAt(Rges.R0()));

	Sys.PC++;
}
void CVir8051::Opcode_97_SUBB_A_R1_1(void) {
//	INT8U temp;
//	void *p = NULL;
//	p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	MD_SUBB(GetRamDataAt(Rges.R1()));

	Sys.PC++;
}
void CVir8051::Opcode_98_SUBB_A_R0(void) {
	MD_SUBB(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_99_SUBB_A_R1(void) {
	MD_SUBB(Rges.R1());
	Sys.PC++;
}
void CVir8051::Opcode_9A_SUBB_A_R2(void) {
	MD_SUBB(Rges.R2());
	Sys.PC++;
}
void CVir8051::Opcode_9B_SUBB_A_R3(void) {
	MD_SUBB(Rges.R3());
	Sys.PC++;
}
void CVir8051::Opcode_9C_SUBB_A_R4(void) {
	MD_SUBB(Rges.R4());
	Sys.PC++;
}
void CVir8051::Opcode_9D_SUBB_A_R5(void) {
	MD_SUBB(Rges.R5());
	Sys.PC++;
}
void CVir8051::Opcode_9E_SUBB_A_R6(void) {
	MD_SUBB(Rges.R6());
	Sys.PC++;
}
void CVir8051::Opcode_9F_SUBB_A_R7(void) {
	MD_SUBB(Rges.R7());
	Sys.PC++;
}
void CVir8051::Opcode_A0_ORL_C_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
//	;
//
//	Assert(temp<0xF7);
//	if (temp <= 0x7F) {
//		data = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		data = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
//	temp = 0;
//	if (Sys.psw().cy) {
//		temp++;
//	}
//	if (!data) //�ѷ�
//	{
//		temp++;
//	}
	Sys.psw().cy = Sys.psw().cy | (!GetBitFlag(temp));

	Sys.PC += 2;
}
void CVir8051::Opcode_A2_MOV_C_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		Sys.psw().cy = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		Sys.psw().cy = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
	Sys.psw().cy = GetBitFlag(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_A3_INC_DPTR(void) {
	Sys.dptr = Sys.dptr() + 1;
	;

	Sys.PC++;
}
void CVir8051::Opcode_A4_MUL_AB(void) {
	INT16U temp;
	temp = (INT16U) Sys.a() * (INT16U) Sys.b();
	Sys.psw().ov = (temp > 255) ? 1 : 0;
	Sys.a = (INT8U) temp;
	Sys.b = (INT8U) (temp >> 8);

	Sys.PC++;
}

void CVir8051::Opcode_A5(void) {
}

void CVir8051::Opcode_A6_MOV_R0_1_Direct(void) {
	INT8U addr;
	INT8U data;
//	void*p0 = NULL;
//	void *p1 = NULL;

//	p0 = GetPointRamAddr(Rges.R0());
	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
//	p1 = GetPointRamAddr(addr);
//	memcpy(&data, p1, 1);
//	memcpy(p0, &data, 1);
	SetRamDataAt(Rges.R0(), data);
	Sys.PC += 2;
}
void CVir8051::Opcode_A7_MOV_R1_1_Direct(void) {
	INT8U addr;
	INT8U data;
	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
	SetRamDataAt(Rges.R1(), data);
	Sys.PC += 2;
}
void CVir8051::Opcode_A8_MOV_R0_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R0 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_A9_MOV_R1_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R1 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AA_MOV_R2_Direct(void) {

	INT8U Addr;
	GetOpcodeData(&Addr, 1);
	Rges.R2 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AB_MOV_R3_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R3 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AC_MOV_R4_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R4 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AD_MOV_R5_Direct(void) {
	INT8U Addr;
	GetOpcodeData(&Addr, 1);
	Rges.R5 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AE_MOV_R6_Direct(void) {
	INT8U Addr;
	GetOpcodeData(&Addr, 1);
	Rges.R6 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AF_MOV_R7_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R7 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_B0_ANL_C_Bit_1(void) {  // ANL C ~Bit
	INT8U temp;
//	INT8U data;
	GetOpcodeData(&temp, 1);
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		data = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		data = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
//	if (Sys.psw().cy) {
//		temp++;
//	}
//
//	if (!data)  //ȡ��
//	{
//		temp++;
//	}
	Sys.psw().cy = Sys.psw().cy & (!GetBitFlag(temp));
	Sys.PC += 2;
}
void CVir8051::Opcode_B2_CPL_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));
//	Assert(temp<=0xF7);
	if (GetBitFlag(temp)) {
		ClrBitFlag(temp);
	} else {
		SetBitFlag(temp);
	}
//	if (temp <= 0x7F) {
//		if (GetBitFlag((temp / 8) + 0x20, temp % 8)) {
//			ClrBitFlag((temp / 8) + 0x20, temp % 8);
//		} else {
//			SetBitFlag((temp / 8) + 0x20, temp % 8);
//		}
//	} else {
//		if (GetBitFlag(ChangeBitAddr(temp), temp % 8)) {
//			ClrBitFlag(ChangeBitAddr(temp), temp % 8);
//		} else {
//			SetBitFlag(ChangeBitAddr(temp), temp % 8);
//		}
//	}
	Sys.PC += 2;
}
void CVir8051::Opcode_B3_CPL_C(void) {
	Sys.psw().cy = ~Sys.psw().cy;
	Sys.PC++;
}

//INT16U CVir8051::GetTargPC(INT8U rel) {
//
//	if (rel < 0x80) {
//		return Sys.PC + rel;
//	} else {
//		return Sys.PC +(char)rel;
//	}
//}
void CVir8051::Opcode_B4_CJNE_A_Data_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Sys.a() != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	if ((Sys.a() < temp[0])) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;
	}
}

void CVir8051::Opcode_B5_CJNE_A_Direct_Rel(void) {
	INT8U temp[2];
	INT8U data;

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	data = GetRamData(temp[0]);
	if (Sys.a() != data) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Sys.a() < data) ? 1 : 0;
}
void CVir8051::Opcode_B6_CJNE_R0_1_Data_Rel(void) {
	INT8U temp[2];
	INT8U data;
	GetOpcodeData(&temp[0], sizeof(temp));
	data = GetRamDataAt(Rges.R0());
	Sys.PC += 3;

	if (data != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (data < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_B7_CJNE_R1_1_Data_Rel(void) {
	INT8U temp[2];
	INT8U data;
	GetOpcodeData(&temp[0], sizeof(temp));
	data = GetRamDataAt(Rges.R1());
	Sys.PC += 3;

	if (data != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (data < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_B8_CJNE_R0_Data_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R0() != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R0() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_B9_CJNE_R1_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Rges.R1() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R1() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BA_CJNE_R2_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R2() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R2() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BB_CJNE_R3_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R3() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R3() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BC_CJNE_R4_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Rges.R4() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R4() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BD_CJNE_R5_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R5() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R5() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BE_CJNE_R6_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R6() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R6() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BF_CJNE_R7_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R7() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R7() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_C0_PUSH_Direct(void) {
	INT8U addr;
	INT8U data = 0;
	GetOpcodeData(&addr, sizeof(addr));
	data = GetRamData(addr);
	Sys.sp = Sys.sp() + 1;
	SetRamDataAt(Sys.sp(), data);
	Sys.PC += 2;
}

void CVir8051::Opcode_C2_CLR_Bit(void) { // λѰַ
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		ClrBitFlag((temp / 8) + 0x20, temp % 8);
//	} else {
//		ClrBitFlag(ChangeBitAddr(temp), temp % 8);
//	}
	ClrBitFlag(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_C3_CLR_C(void) {
	Sys.psw().cy = 0;
	Sys.PC++;
}
void CVir8051::Opcode_C4_SWAP_A(void) {
	INT8U temp = Sys.a() & 0x0F;
	Sys.a = (Sys.a() >> 4) | (temp << 4);
	Sys.PC++;
}
void CVir8051::Opcode_C5_XCH_A_Direct(void) {
	INT8U addr;
	INT8U data;
	INT8U TT;
	GetOpcodeData(&addr, sizeof(addr));
	data = GetRamData(addr);
	TT = Sys.a();
	Sys.a = data;
	SetRamData(addr, TT);
	Sys.PC += 2;
}
void CVir8051::Opcode_C6_XCH_A_R0_1(void) {
	INT8U data;
	INT8U TT;
	data = GetRamDataAt(Rges.R0());
	TT = Sys.a();
	Sys.a = data;
	SetRamDataAt(Rges.R0(), TT);
	Sys.PC++;
}
void CVir8051::Opcode_C7_XCH_A_R1_1(void) {
	INT8U data;
	INT8U TT;
	data = GetRamDataAt(Rges.R1());
	TT = Sys.a();
	Sys.a = data;
	SetRamDataAt(Rges.R1(), TT);
	Sys.PC++;
}
void CVir8051::Opcode_C8_XCH_A_R0(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R0();
	Rges.R0 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_C9_XCH_A_R1(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R1();
	Rges.R1 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CA_XCH_A_R2(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R2();
	Rges.R2 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CB_XCH_A_R3(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R3();
	Rges.R3 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CC_XCH_A_R4(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R4();
	Rges.R4 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CD_XCH_A_R5(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R5();
	Rges.R5 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CE_XCH_A_R6(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R6();
	Rges.R6 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CF_XCH_A_R7(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R7();
	Rges.R7 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_D0_POP_Direct(void) {
	INT8U addr;
	INT8U data;
	GetOpcodeData(&addr, sizeof(addr));
	data = GetRamDataAt(Sys.sp());
	SetRamData(addr, data);
	Sys.sp = Sys.sp() - 1;
	Sys.PC += 2;
}

void CVir8051::Opcode_D2_SETB_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));  //bit
	SetBitFlag(temp);
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		SetBitFlag((temp / 8) + 0x20, temp % 8);
//	} else {
//		SetBitFlag(ChangeBitAddr(temp), temp % 8);
//	}
	Sys.PC += 2;
}
void CVir8051::Opcode_D3_SETB_C(void) {
	Sys.psw().cy = 1;
	Sys.PC++;
}
void CVir8051::Opcode_D4_DA_A(void) {
	INT8U temp;
	if (((Sys.a() & 0x0f) > 9) || (Sys.psw().ac == 1)) {
		temp = ((Sys.a() & 0x0f) + 6) % 16;
		Sys.a = Sys.a() & 0xF0;
		Sys.a = Sys.a() | temp;
	}
	temp = ((Sys.a() & 0xF0) >> 4);
	if ((temp > 9) || (Sys.psw().cy == 1)) {
		temp = (temp + 6) % 16;
		Sys.a = Sys.a() & 0x0F;
		Sys.a = Sys.a() | (temp << 4);
	}
	Sys.PC++;
}
void CVir8051::Opcode_D5_DJNZ_Direct_Rel(void) {
	INT8U temp[2];
	INT8U data;

	GetOpcodeData(&temp[0], sizeof(temp));

	data = GetRamData(temp[0]);

	data--;

	SetRamData(temp[0], data);

	Sys.PC += 3;
	if (data != 0) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
}
void CVir8051::Opcode_D6_XCHD_A_R0_1(void) {
	INT8U temp = 0;
	INT8U data = GetRamDataAt(Rges.R0());
	temp = Sys.a() & 0x0F;
	Sys.a = Sys.a() & 0xF0;
	Sys.a = Sys.a() | (data & 0x0F);
	data &= 0xF0;
	data |= temp;
	SetRamDataAt(Rges.R0(), data);
	Sys.PC++;
}
void CVir8051::Opcode_D7_XCHD_A_R1_1(void) {
	INT8U temp = 0;
	INT8U data;
	data = GetRamDataAt(Rges.R1());
	temp = Sys.a() & 0x0F;
	Sys.a = Sys.a() & 0xF0;
	Sys.a = Sys.a() | (data & 0x0F);
	data &= 0xF0;
	data |= temp;
	SetRamDataAt(Rges.R1(), data);
	Sys.PC++;
}
void CVir8051::Opcode_D8_DJNZ_R0_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R0 = Rges.R0() - 1;
	if (Rges.R0() != 0) {
		// pc(0440+2)+temp(FD)=043f
		//Sys.PC+=temp;
//		Sys.PC = GetTargPC(temp);

		Sys.PC += temp;
	}
}
void CVir8051::Opcode_D9_DJNZ_R1_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R1 = Rges.R1() - 1;
	if (Rges.R1() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DA_DJNZ_R2_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R2 = Rges.R2() - 1;
	if (Rges.R2() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DB_DJNZ_R3_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R3 = Rges.R3() - 1;
	if (Rges.R3() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DC_DJNZ_R4_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R4 = Rges.R4() - 1;
	if (Rges.R4() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DD_DJNZ_R5_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R5 = Rges.R5() - 1;
	if (Rges.R5() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DE_DJNZ_R6_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R6 = Rges.R6() - 1;
	if (Rges.R6() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DF_DJNZ_R7_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R7 = Rges.R7() - 1;
	if (Rges.R7() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_E0_MOVX_A_DPTR(void) {
	INT8U temp = m_ExRam[Sys.dptr()];
//	void*p = GetExRamAddr(Sys.dptr());
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = temp;
	Sys.PC++;
}
void CVir8051::Opcode_E2_MOVX_A_R0_1(void) {
	INT8U temp = m_ExRam[Rges.R0()];
	Sys.a = temp;
	Sys.PC++;
}
void CVir8051::Opcode_E3_MOVX_A_R1_1(void) {
	INT8U temp = m_ExRam[Rges.R1()];
	Sys.a = temp;
	Sys.PC++;
}
void CVir8051::Opcode_E4_CLR_A(void) {
	Sys.a = 0;
	Sys.PC++;
}
void CVir8051::Opcode_E5_MOV_A_Direct(void) {
	INT8U addr;
	GetOpcodeData(&addr, sizeof(addr));
	Sys.a = GetRamData(addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_E6_MOV_A_R0_1(void) {
	Sys.a = GetRamDataAt(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_E7_MOV_A_R1_1(void) {
	Sys.a = GetRamDataAt(Rges.R1());
	Sys.PC++;
}
void CVir8051::Opcode_E8_MOV_A_R0(void) {
	Sys.a = Rges.R0();

	Sys.PC++;
}
void CVir8051::Opcode_E9_MOV_A_R1(void) {
	Sys.a = Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_EA_MOV_A_R2(void) {
	Sys.a = Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_EB_MOV_A_R3(void) {
	Sys.a = Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_EC_MOV_A_R4(void) {
	Sys.a = Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_ED_MOV_A_R5(void) {
	Sys.a = Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_EE_MOV_A_R6(void) {
	Sys.a = Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_EF_MOV_A_R7(void) {
	Sys.a = Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_F0_MOVX_DPTR_A(void) {
//	INT8U data;
//	void*p = GetExRamAddr(Sys.dptr());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetExRam(Sys.dptr(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F2_MOVX_R0_1_A(void) {
//	INT8U data;
//	void*p = GetExRamAddr(Rges.R0());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetExRam(Rges.R0(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F3_MOVX_R1_1_A(void) {
//	INT8U data;
//	void*p = GetExRamAddr(Rges.R1());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetExRam(Rges.R1(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F4_CPL_A(void) {
	Sys.a = ~(Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F5_MOV_Direct_A(void) {
	INT8U addr;
//	INT8U data;
//	void * p = NULL;

	GetOpcodeData(&addr, sizeof(addr));
//	p = GetPointRamAddr(addr);
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetRamData(addr, Sys.a());
	Sys.PC += 2;
}
void CVir8051::Opcode_F6_MOV_R0_1_A(void) {
//	INT8U addr;
//	void *p = NULL;

//	p = GetPointRamAddr(Rges.R0());
//	addr = Sys.a();
//	memcpy(p, &addr, sizeof(addr));
	SetRamDataAt(Rges.R0(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F7_MOV_R1_1_A(void) {
//	INT8U data;
//	void *p = NULL;
//
//	p = GetPointRamAddr(Rges.R1());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetRamDataAt(Rges.R1(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F8_MOV_R0_A(void) {
	Rges.R0 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_F9_MOV_R1_A(void) {
	Rges.R1 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FA_MOV_R2_A(void) {
	Rges.R2 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FB_MOV_R3_A(void) {
	Rges.R3 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FC_MOV_R4_A(void) {
	Rges.R4 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FD_MOV_R5_A(void) {
	Rges.R5 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FE_MOV_R6_A(void) {
	Rges.R6 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FF_MOV_R7_A(void) {
	Rges.R7 = Sys.a();
	Sys.PC++;
}
shared_ptr<vector<unsigned char>> CVir8051::GetRetData(void) const {
	auto tem = make_shared<vector<unsigned char>>();

	char buffer[1024] = { 0 };
	memcpy(buffer, &m_ExRam[0xFC00], 1023);
	tem.get()->assign(buffer, buffer + 1023);
	return tem;
}

//
//INT8U& CUPReg_a::GetRegRe(void) {
//	Assert(m_Addr != 255);
//	return pmcu->m_ChipRam[m_Addr];
//}
template<class T2>
T2& CUPReg<T2>::GetRegRe(void) {
	assert(m_Addr != 255);
	return *((T2*) (&pmcu->m_ChipRam[m_Addr]));
}
INT8U& CUPReg_a::GetRegRe(void) {
	assert(m_Addr != 255);
	return pmcu->m_ChipSfr[m_Addr];
}

INT8U& CUPSfr::GetRegRe(void) {
	assert(m_Addr != 255);
	return pmcu->m_ChipSfr[m_Addr];
}
INT16U& CUPSfr16::GetRegRe(void) {
	assert(m_Addr != 255);
	return *((INT16U*) &pmcu->m_ChipSfr[m_Addr]);
}

template<class T2>
T2 CUPReg<T2>::getValue() {
	return GetRegRe();
}

void CUPReg_a::Updataflag() {
	pmcu->Updata_A_P_Flag();
}

PSW& CUPPSW_8::operator()(void) {
	return *((PSW*) &(GetRegRe()));
}

