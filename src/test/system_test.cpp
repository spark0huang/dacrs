#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "rpcclient.h"
#include "tx.h"
#include "wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "./VmScript/VmScript.h"
#include "rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "SysTestBase.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost;

#define ACCOUNT_ID_SIZE 6
#define MAX_ACCOUNT_LEN 20
#pragma pack(1)
typedef struct tag_INT64 {
	unsigned char data[8];
} Int64;
typedef struct tagACCOUNT_ID
{
	char accounid[MAX_ACCOUNT_LEN];
}ACCOUNT_ID;
typedef struct {
	unsigned char nType;
	ACCOUNT_ID vregID[3];
	long nHeight;
	Int64 nPay;
} CONTRACT_DATA;
#pragma pack()

class CSystemTest:public SysTestBase
{
public:
	enum
	{
		ID1_FREE_TO_ID2_FREE = 1,
		ID2_FREE_TO_ID3_FREE,
		ID3_FREE_TO_ID3_SELF,
		ID3_SELF_TO_ID2_FREE,
		ID3_FREE_TO_ID2_FREE,
		UNDEFINED_OPER
	};
	CSystemTest() {
		nOldBlockHeight = 0;
		nNewBlockHeight = 0;
		nTimeOutHeight = 100;
		nPayMoney = 10000;
		nOldMoney = 0;
		nNewMoney = 0;
		nRandomRanger = 10000;
		strFileName = "RegScriptTest.bin";
		strAddr1 = "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA";
		strAddr3 = "mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7";
		strRegID1 = "000000000900";
		strRegID2 = "000000000500";
		strRegID3 = "000000000700";
	}

	~CSystemTest(){

	}

public:
	int GetBlockHeight()
	{
		return  (int)chainActive.Height();
	}

	bool GetBlockHash(int nHeight,uint256& blockHash)
	{
		if (nHeight < 0 || nHeight > chainActive.Height())
			return false;

		CBlockIndex* pblockindex = chainActive[nHeight];
		blockHash = pblockindex->GetBlockHash();
		return true;
	}

	bool IsTxConfirmdInWallet(int nBlockHeight,const uint256& txHash)
	{
		uint256 blockHash;
		if (!GetBlockHash(nBlockHeight, blockHash)) {
			return false;
		}

		auto itAccountTx = pwalletMain->mapInBlockTx.find(blockHash);
		if (pwalletMain->mapInBlockTx.end() == itAccountTx)
			return false;

		for (const auto &item :itAccountTx->second.mapAccountTx) {
			if (txHash == item.first) {
				return true;
			}
		}
		return false;
	}

	bool IsTxUnConfirmdInWallet(const uint256& txHash) {
		for (const auto &item : pwalletMain->UnConfirmTx) {
			if (txHash == item.first) {
				return true;
			}
		}
		return false;
	}

	bool IsTxInMemorypool(const uint256& txHash) {
		for (const auto& entry : mempool.mapTx) {
			if (entry.first == txHash)
				return true;
		}

		return false;
	}

	bool GetTxIndexInBlock(const uint256& txHash, int& nIndex) {
		CBlockIndex* pindex = chainActive.Tip();
		CBlock block;
		if (!ReadBlockFromDisk(block, pindex))
			return false;

		block.BuildMerkleTree();
		std::tuple<bool,int> ret = block.GetTxIndex(txHash);
		if (!std::get<0>(ret)) {
			return false;
		}

		nIndex = std::get<1>(ret);
		return true;
	}

	bool GetRegScript(map<string, string>& mapRegScript) {
		CRegID regId;
		vector<unsigned char> vScript;

		if (pScriptDBTip == nullptr)
			return false;

		assert(pScriptDBTip->Flush());

		int nCount(0);
		if (!pScriptDBTip->GetScriptCount(nCount))
			return false;

		if (!pScriptDBTip->GetScript(0, regId, vScript))
			return false;

		string strRegID = HexStr(regId.GetVec6());
		string strScript = HexStr(vScript.begin(), vScript.end());
		mapRegScript.insert(make_pair(strRegID, strScript));

		while (pScriptDBTip->GetScript(1, regId, vScript)) {
			strRegID = HexStr(regId.GetVec6());
			strScript = HexStr(vScript.begin(), vScript.end());
			mapRegScript.insert(make_pair(strRegID, strScript));
		}

		return true;
	}

	bool CheckRegScript(const string& strRegID,const string& strPath) {
		map<string, string> mapRegScript;
		if (!GetRegScript(mapRegScript)) {
			return false;
		}

		string strFileData;
		if (!GetFileData(strPath,strFileData)) {
			return false;
		}

		for (const auto& item:mapRegScript) {
			if (strRegID == item.first) {
				if (strFileData == item.second) {
					return true;
				}
			}
		}

		return false;
	}

	bool GetFileData(const string& strFilePath, string& strFileData) {
		FILE* file = fopen(strFilePath.c_str(), "rb+");
		if (!file) {
			return false;
		}

		unsigned long lSize;
		fseek(file, 0, SEEK_END);
		lSize = ftell(file);
		rewind(file);

		// allocate memory to contain the whole file:
		char *buffer = (char*) malloc(sizeof(char) * lSize);
		if (buffer == NULL) {
			return false;
		}

		if (fread(buffer, 1, lSize, file) != lSize) {
			if (buffer)
				free(buffer);
			throw runtime_error("read script file error");
		}

		CVmScript vmScript;
		vmScript.Rom.insert(vmScript.Rom.end(), buffer, buffer + lSize);
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << vmScript;

		vector<unsigned char> vscript;
		vscript.assign(ds.begin(), ds.end());

		if (file)
			fclose(file);
		if (buffer)
			free(buffer);

		strFileData = HexStr(vscript);
		return true;
	}

	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type){
			return false;
		}

		strHash = result.get_str();
		return true;
	}

	bool GetTxOperateLog(const uint256& txHash, vector<CAccountOperLog>& vLog) {
		if (!GetTxOperLog(txHash, vLog))
			return false;

		return true;
	}

	bool PacketContractData(unsigned char nOperType,const string& strBuyerID,const string& strSellerID,const string& strAr,
			int nHeight,int nMoney,string& strData) {
		if (nOperType>=UNDEFINED_OPER || strBuyerID.size() != 2*ACCOUNT_ID_SIZE|| strSellerID.size() != 2*ACCOUNT_ID_SIZE) {
					return false;
				}
		memset(&contractData,0,sizeof(CONTRACT_DATA));
		contractData.nType = nOperType;
		vector<unsigned char> v = ParseHex(strBuyerID.c_str());
		memcpy(contractData.vregID[0].accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(strSellerID.c_str());
		memcpy(contractData.vregID[1].accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(strAr.c_str());
		memcpy(contractData.vregID[2].accounid,&v[0],ACCOUNT_ID_SIZE);

		contractData.nHeight = nHeight;

		memcpy(contractData.nPay.data,&nMoney,sizeof(nMoney));

		string strContractData;
		char* pData = (char*)&contractData;
		strData.assign(pData,pData+sizeof(contractData));
		return true;
	}

	bool GenerateOneBlock() {
		nOldBlockHeight = GetBlockHeight();
		if (!SysTestBase::GenerateOneBlock()) {
			return false;
		}

		nNewBlockHeight = GetBlockHeight();
		if (nNewBlockHeight != nOldBlockHeight + 1) {
			return false;
		}

		return true;
	}

	bool IsScriptAccCreatedEx(const uint256& txHash,int nConfirmHeight) {
		int nIndex = 0;
		if (!GetTxIndexInBlock(uint256(strTxHash), nIndex)) {
			return false;
		}

		CRegID regID(nConfirmHeight, nIndex);
		return IsScriptAccCreated(HexStr(regID.GetVec6()));
	}

	int GetRandomValue()
	{
		return rand()%nRandomRanger+1;
	}

	void SetRandomRanger(int nMaxRanger) {
		if (!nMaxRanger) {
			nMaxRanger = 10000;
		}
		nRandomRanger = nMaxRanger;
	}

	CAuthorizate GetAuthorByAddress(const string& strAddress, const string& strScriptID) {
		CDacrsAddress address(strAddress);
		CKeyID keyID;
		BOOST_CHECK(address.GetKeyID(keyID));
		CUserID userId = keyID;
		CAccount account;
		CRegID scriptId(strScriptID);
		CAuthorizate authorizate;
		BOOST_CHECK(pAccountViewTip->GetAccount(userId, account));
		BOOST_CHECK(pScriptDBTip->GetAuthorizate(account.regID, scriptId, authorizate));

		return std::move(authorizate);
	}

	void Transfer_Authorizated(const string& strScriptID, const string& strSignAddr, const string& strOperAddr,
			unsigned char nTestType) {
		string strContractData;
		string vAddr = "[\"" + strSignAddr + "\"] ";

		CAuthorizate author = GetAuthorByAddress(strOperAddr, strScriptID);
		unsigned int nAvailable = author.GetCurMaxMoneyPerDay();
		SetRandomRanger(nAvailable);
		unsigned int nRandomValue = 0;

		for (int i = 0; i < nRandomTestCount; i++) {
			nRandomValue = GetRandomValue();
			BOOST_CHECK(PacketContractData(nTestType, strRegID1, strRegID2, //
					strRegID3, nTimeOutHeight, nRandomValue, strContractData));

			nOldMoney = GetFreeMoney(strOperAddr);
			if (nOldMoney >= nRandomValue) {
				if (nAvailable >= nRandomValue) {
					nAvailable -= nRandomValue;
					BOOST_CHECK(CreateContractTx(strScriptID, vAddr, strContractData, nTimeOutHeight, nFee));
					BOOST_CHECK(GenerateOneBlock());
					nNewMoney = GetFreeMoney(strOperAddr);
					BOOST_CHECK(nOldMoney - nRandomValue == nNewMoney);
//					cout << "random money is " << nRandomValue << " old money: " << nOldMoney << " new money is "
//							<< nNewMoney << endl;
				} else {
					BOOST_CHECK(!CreateContractTx(strScriptID, vAddr, strContractData, nTimeOutHeight, nFee));
				}

			}
		}
	}

	void Transfer_NotSigned_NotAuthor(const string& strScriptID, const string& strSignAddr, unsigned char nTestType) {
		string strContractData;
		int nRandomValue = 0;
		string vAddr = "[\"" + strSignAddr + "\"] ";
		for (int i = 0; i < nRandomTestCount; i++) {
			nRandomValue = GetRandomValue();
			BOOST_CHECK(PacketContractData(nTestType, strRegID1, strRegID2, //
					strRegID3, nTimeOutHeight, nRandomValue, strContractData));

			BOOST_CHECK(!CreateContractTx(strScriptID, vAddr, strContractData, nTimeOutHeight, nFee));

		}
	}

	void Transfer_Signed(const string& strScriptID, const string& strSignAddr,unsigned char nTestType) {
		string strContractData;
		string vAddr = "[\"" + strSignAddr + "\"] ";
		unsigned int nRandomValue = 0;
		for (int i = 0; i < nRandomTestCount; i++) {
			nRandomValue = GetRandomValue();
			BOOST_CHECK(PacketContractData(nTestType, strRegID1, strRegID2, //
					strRegID3, nTimeOutHeight, nRandomValue, strContractData));

			nOldMoney = GetFreeMoney(strSignAddr);
			if (nOldMoney >= nRandomValue) {
				BOOST_CHECK(CreateContractTx(strScriptID, vAddr, strContractData, nTimeOutHeight, nFee));
				BOOST_CHECK(GenerateOneBlock());
				nNewMoney = GetFreeMoney(strSignAddr);
				BOOST_CHECK(nOldMoney - nRandomValue - nFee == nNewMoney);
//				cout << "random money is " << nRandomValue << " old money: " << nOldMoney << " new money is "
//						<< nNewMoney << endl;
			}
		}
	}

protected:
	int nRandomRanger;
	int nOldBlockHeight;
	int nNewBlockHeight;
	int nTimeOutHeight;
	int nPayMoney;
	static const int nFee = 100000;
	static const int nRandomTestCount = 20;
	uint64_t nOldMoney;
	uint64_t nNewMoney;
	string strTxHash;
	string strFileName;
	string strAddr1;
	string strAddr3;
	string strRegID1;
	string strRegID2;
	string strRegID3;
	CONTRACT_DATA contractData;
};

BOOST_FIXTURE_TEST_SUITE(system_test,CSystemTest)
BOOST_FIXTURE_TEST_CASE(reg_test,CSystemTest)
{
//	int nOldBlockHeight = 0;
//	int nNewBlockHeight = 0;
//	int nTimeOutHeight = 5;
//	int nFee = 100000;
//	uint64_t nOldMoney = 0;
//	uint64_t nNewMoney = 0;
//	string strTxHash;
//	string strFileName("RegScriptTest.bin");
//	string strAddr("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA");

	vector<map<int,string> >vDataInfo;
	vector<CAccountOperLog> vLog;
	for (int i = 0; i < nTimeOutHeight; i++) {
		//0:����ע��ű�����
		Value valueRes = RegisterScriptTx(strAddr1,strFileName , nTimeOutHeight, nFee);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));

		//1:�ڿ�
		nOldMoney = GetFreeMoney(strAddr1);
		BOOST_CHECK(GenerateOneBlock());

		//2:ȷ��Ǯ�Ѿ��۳�
		nNewMoney = GetFreeMoney(strAddr1);
		BOOST_CHECK(nNewMoney == nOldMoney - nFee);

		//3:ȷ�Ͻű��˺��Ѿ�����
		int nIndex = 0;
		BOOST_CHECK(GetTxIndexInBlock(uint256(strTxHash), nIndex));
		CRegID regID(nNewBlockHeight, nIndex);
		BOOST_CHECK(IsScriptAccCreated(HexStr(regID.GetVec6())));

		//4:���Ǯ�������ȷ�Ͻ������Ƿ��д˱ʽ���
		BOOST_CHECK(IsTxConfirmdInWallet(nNewBlockHeight, uint256(strTxHash)));

		//5:ͨ��listregscript ��ȡ�����Ϣ��һһ�˶ԣ����Ƿ�������һ��
		string strPath = SysCfg().GetDefaultTestDataPath() + strFileName;
		BOOST_CHECK(CheckRegScript(HexStr(regID.GetVec6()), strPath));

		//6:Gettxoperationlog ��ȡ����log���鿴�Ƿ���ȷ
		BOOST_CHECK(GetTxOperateLog(uint256(strTxHash), vLog));
		BOOST_CHECK(1 == vLog.size() && 1 == vLog[0].vOperFund.size() && 1 == vLog[0].vOperFund[0].vFund.size());
		BOOST_CHECK(strAddr1 == vLog[0].keyID.ToAddress());
		BOOST_CHECK(vLog[0].vOperFund[0].operType == MINUS_FREE && vLog[0].vOperFund[0].vFund[0].value == nFee);

		map<int,string> mapData;
		mapData.insert(make_pair(nIndex,strTxHash));
		vDataInfo.push_back(std::move(mapData));
	}

	for(int i = vDataInfo.size()-1;i>=0;i--) {
		map<int,string> mapData = vDataInfo[i];
		BOOST_CHECK(1 == mapData.size());

//		int nTxIndex = mapData.begin()->first;
		string strTxHash = mapData.begin()->second;
		uint256 txHash(strTxHash);

		nOldBlockHeight = GetBlockHeight();
		nOldMoney = GetFreeMoney(strAddr1);

		//8:�ع�
		BOOST_CHECK(DisConnectBlock(1));

		//9.1:����˻��������Ƿ����
		nNewMoney = GetFreeMoney(strAddr1);
		nNewBlockHeight = GetBlockHeight();
		BOOST_CHECK(nOldBlockHeight - 1 == nNewBlockHeight);
		BOOST_CHECK(nNewMoney-nFee == nOldMoney);

		//9.2:���ű��˻��Ƿ�ɾ��
		CRegID regID(nOldBlockHeight, mapData.begin()->first);
		BOOST_CHECK(!IsScriptAccCreated(HexStr(regID.GetVec6())));

		//9.3:�����Ƿ��Ѿ��Ѿ��ŵ�Ǯ����δȷ�Ͻ�����
		BOOST_CHECK(IsTxUnConfirmdInWallet(txHash));

		//9.4:��齻���Ƿ���mempool��
		BOOST_CHECK(IsTxInMemorypool(txHash));

		//9.5:���operationlog �Ƿ�������»�ȡ
		BOOST_CHECK(!GetTxOperateLog(txHash, vLog));
	}

	//��ջ���
	ResetEnv();
	nNewBlockHeight = GetBlockHeight();
	BOOST_CHECK(0 == nNewBlockHeight);
}

BOOST_FIXTURE_TEST_CASE(author_test,CSystemTest)
{
	//��ջ���
	ResetEnv();
	nNewBlockHeight = GetBlockHeight();
	BOOST_CHECK(0 == nNewBlockHeight);

	//0:�����ű�����
	Value valueRes = RegisterScriptTx(strAddr1, strFileName, nTimeOutHeight, nFee);
	BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));

	//1:�ڿ�
	BOOST_CHECK(GenerateOneBlock());

	//�޸�Ȩ��(�Ե�һ���͵������˺���Ȩ)
	//�˺�1��ǩ������Ȩ �˺�2����ǩ������Ȩ �˺�3����ǩ������Ȩ
	int nIndex = 0;
	BOOST_CHECK(GetTxIndexInBlock(uint256(strTxHash), nIndex));
	CRegID striptID(nNewBlockHeight, nIndex);

	vector<unsigned char> vUserDefine;
	string strHash1,strHash2;
	vUserDefine.push_back(1);
	const int nMaxMoneyPerDay = 15000;
	CNetAuthorizate author(100,vUserDefine,10000,nMaxMoneyPerDay,nMaxMoneyPerDay);
	valueRes = ModifyAuthor(strAddr1,HexStr(striptID.GetVec6()),nTimeOutHeight,nFee,author);
	BOOST_CHECK(GetHashFromCreatedTx(valueRes, strHash1));

	valueRes = ModifyAuthor(strAddr3,HexStr(striptID.GetVec6()),nTimeOutHeight,nFee,author);
	BOOST_CHECK(GetHashFromCreatedTx(valueRes, strHash2));
	BOOST_CHECK(GenerateOneBlock());
	nNewBlockHeight = GetBlockHeight();

	BOOST_CHECK(IsTxConfirmdInWallet(nNewBlockHeight, uint256(strHash1)));
	BOOST_CHECK(IsTxConfirmdInWallet(nNewBlockHeight, uint256(strHash2)));

	//��Լ���ף���������һ����Ǯ���鿴�Ƿ������Ȩ�޽������
	string strScriptID(HexStr(striptID.GetVec6()));
	string vconaddr = "[\"" + strAddr1 + "\"] ";
	string strContractData;
	int nAvailable = nMaxMoneyPerDay;
	BOOST_CHECK(PacketContractData(ID3_FREE_TO_ID3_SELF, strRegID1, strRegID2, //
			strRegID3, nTimeOutHeight, 5000, strContractData));
	BOOST_CHECK(CreateContractTx(strScriptID, vconaddr, strContractData, nTimeOutHeight, nFee));
	BOOST_CHECK(GenerateOneBlock());
	nAvailable -= nPayMoney;

	//�۳����������е�Ǯ�������Ȩ��
	BOOST_CHECK(nAvailable < nPayMoney);
	BOOST_CHECK(PacketContractData(ID3_SELF_TO_ID2_FREE, strRegID1, strRegID2, //
			strRegID3, nTimeOutHeight, 5000, strContractData));
	BOOST_CHECK(CreateContractTx(strScriptID, vconaddr, strContractData, nTimeOutHeight, nFee));
	BOOST_CHECK(GenerateOneBlock());
	nAvailable -= nPayMoney;

	//�ٴο۳�������ʣ��Ȩ�ޣ����ɽ������Ȩ��
	BOOST_CHECK(nAvailable < nPayMoney);
	BOOST_CHECK(PacketContractData(ID3_FREE_TO_ID3_SELF, strRegID1, strRegID2, //
				strRegID3, nTimeOutHeight, 10000, strContractData));
	BOOST_CHECK(!CreateContractTx(strScriptID, vconaddr, strContractData, nTimeOutHeight, nFee));
	BOOST_CHECK(GenerateOneBlock());

	vector<uint256> vUnConfirmTxHash;
	//����ǩ���˻����ת��
	Transfer_Signed(strScriptID,strAddr1,ID1_FREE_TO_ID2_FREE);

	//��δǩ������Ȩ�˻��������
	Transfer_Authorizated(strScriptID,strAddr1,strAddr3,ID3_FREE_TO_ID2_FREE);

	//��δǩ��δ��Ȩ�˻��������
	Transfer_NotSigned_NotAuthor(strScriptID,strAddr1,ID2_FREE_TO_ID3_FREE);
}
BOOST_AUTO_TEST_SUITE_END()

