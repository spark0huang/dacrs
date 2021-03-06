#include "account.h"
#include "util.h"
#include "serialize.h"
#include "core.h"
#include "main.h"
#include "chainparams.h"
#include <algorithm>

bool CAccountView::GetAccount(const CKeyID &keyId, CAccount &account) {return false;}
bool CAccountView::SetAccount(const CKeyID &keyId, const CAccount &account) {return false;}
bool CAccountView::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {return false;}
bool CAccountView::HaveAccount(const CKeyID &keyId) {return false;}
uint256 CAccountView::GetBestBlock() {return false;}
bool CAccountView::SetBestBlock(const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlock) {return false;}
bool CAccountView::BatchWrite(const vector<CAccount> &vAccounts) {return false;}
bool CAccountView::EraseAccount(const CKeyID &keyId) {return false;}
bool CAccountView::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {return false;}
bool CAccountView::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {return false;}
bool CAccountView::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {return false;}
bool CAccountView::EraseKeyId(const vector<unsigned char> &accountId){
	return false;
}
bool CAccountView::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId, const CAccount &account) {return false;}
Object CAccountView::ToJosnObj(char Prefix){
	Object obj;
	return obj;
}

CAccountViewBacked::CAccountViewBacked(CAccountView &accountView):pBase(&accountView) {}
bool CAccountViewBacked::GetAccount(const CKeyID &keyId, CAccount &account) {
	return pBase->GetAccount(keyId, account);
}
bool CAccountViewBacked::SetAccount(const CKeyID &keyId, const CAccount &account) {
	return pBase->SetAccount(keyId, account);
}
bool CAccountViewBacked::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {
	return pBase->SetAccount(accountId, account);
}
bool CAccountViewBacked::HaveAccount(const CKeyID &keyId) {
	return pBase->HaveAccount(keyId);
}
uint256 CAccountViewBacked::GetBestBlock() {
	return pBase->GetBestBlock();
}
bool CAccountViewBacked::SetBestBlock(const uint256 &hashBlock) {
	return pBase->SetBestBlock(hashBlock);
}
bool CAccountViewBacked::BatchWrite(const map<CKeyID , CAccount> &mapAccounts, const map<std::vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlock) {
	return pBase->BatchWrite(mapAccounts, mapKeyIds, hashBlock);
}
bool CAccountViewBacked::BatchWrite(const vector<CAccount> &vAccounts) {
	return pBase->BatchWrite(vAccounts);
}
bool CAccountViewBacked::EraseAccount(const CKeyID &keyId) {
	return pBase->EraseAccount(keyId);
}
bool CAccountViewBacked::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	return pBase->SetKeyId(accountId, keyId);
}
bool CAccountViewBacked::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	return pBase->GetKeyId(accountId, keyId);
}
bool CAccountViewBacked::EraseKeyId(const vector<unsigned char> &accountId){
	return pBase->EraseKeyId(accountId);
}
bool CAccountViewBacked::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {
	return pBase->GetAccount(accountId, account);
}
bool CAccountViewBacked::SaveAccountInfo(const vector<unsigned char> &accountId, const CKeyID &keyId,
		const CAccount &account) {
	return pBase->SaveAccountInfo(accountId, keyId, account);
}


CAccountViewCache::CAccountViewCache(CAccountView &accountView, bool fDummy):CAccountViewBacked(accountView), hashBlock(0) {}
bool CAccountViewCache::GetAccount(const CKeyID &keyId, CAccount &account) {
	if (cacheAccounts.count(keyId)) {
		if (cacheAccounts[keyId].keyID != uint160(0)) {
			account = cacheAccounts[keyId];
			return true;
		} else
			return false;
	}
	if (pBase->GetAccount(keyId, account)) {
		cacheAccounts[keyId] = account;
		return true;
	}
	return false;
}
bool CAccountViewCache::SetAccount(const CKeyID &keyId, const CAccount &account) {
	cacheAccounts[keyId] = account;
	cacheAccounts[keyId].accountOperLog.SetNULL();
	return true;
}
bool CAccountViewCache::SetAccount(const vector<unsigned char> &accountId, const CAccount &account) {
	if(cacheKeyIds.count(accountId)) {
		cacheAccounts[cacheKeyIds[accountId]] = account;
		cacheAccounts[cacheKeyIds[accountId]].accountOperLog.SetNULL();
		return true;
	}
	return false;
}
bool CAccountViewCache::HaveAccount(const CKeyID &keyId) {
	if(cacheAccounts.count(keyId))
		return true;
	else
		return pBase->HaveAccount(keyId);
}
uint256 CAccountViewCache::GetBestBlock() {
	if(hashBlock == uint256(0))
		return pBase->GetBestBlock();
	return hashBlock;
}
bool CAccountViewCache::SetBestBlock(const uint256 &hashBlockIn) {
	hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const map<CKeyID, CAccount> &mapAccounts, const map<vector<unsigned char>, CKeyID> &mapKeyIds, const uint256 &hashBlockIn) {
	for (map<CKeyID, CAccount>::const_iterator it = mapAccounts.begin(); it != mapAccounts.end(); ++it) {
		if (uint160(0) == it->second.keyID) {
			pBase->EraseAccount(it->first);
			cacheAccounts.erase(it->first);
		} else {
			cacheAccounts[it->first] = it->second;
			cacheAccounts[it->first].accountOperLog.SetNULL();
		}
	}

    for	(map<vector<unsigned char>, CKeyID>::const_iterator itKeyId = mapKeyIds.begin(); itKeyId != mapKeyIds.end(); ++itKeyId)
    	cacheKeyIds[itKeyId->first] = itKeyId->second;
    hashBlock = hashBlockIn;
	return true;
}
bool CAccountViewCache::BatchWrite(const vector<CAccount> &vAccounts) {
	for (vector<CAccount>::const_iterator it = vAccounts.begin(); it != vAccounts.end(); ++it) {
		if (it->keyID.ToString() == "4fb64dd1d825bb6812a7090a1d0dd2c75b55242e") {
			LogPrint("INFO", "get account info:%s\n", it->ToString());
		}
		if (it->IsEmptyValue() && !it->IsRegister()) {
			cacheAccounts[it->keyID] = *it;
			cacheAccounts[it->keyID].keyID = uint160(0);
			cacheAccounts[it->keyID].accountOperLog.SetNULL();
		} else {
			cacheAccounts[it->keyID] = *it;
			cacheAccounts[it->keyID].accountOperLog.SetNULL();
		}
	}
	return true;
}
bool CAccountViewCache::EraseAccount(const CKeyID &keyId) {
	if(cacheAccounts.count(keyId))
		cacheAccounts[keyId].keyID = uint160(0);
	else {
		CAccount account;
		if(pBase->GetAccount(keyId, account)) {
			account.keyID = uint160(0);
			cacheAccounts[keyId] = account;
		}
	}
	return true;
}
bool CAccountViewCache::SetKeyId(const vector<unsigned char> &accountId, const CKeyID &keyId) {
	cacheKeyIds[accountId] = keyId;
	return true;
}
bool CAccountViewCache::GetKeyId(const vector<unsigned char> &accountId, CKeyID &keyId) {
	if(cacheKeyIds.count(accountId)){
		keyId = cacheKeyIds[accountId];
		if(keyId != uint160(0))
		{
			return true;
		}
		else {
			return false;
		}
	}
	if(pBase->GetKeyId(accountId, keyId) ){
		cacheKeyIds[accountId] = keyId;
		return true;
	}
	return false;
}

bool CAccountViewCache::EraseKeyId(const vector<unsigned char> &accountId) {
	if (cacheKeyIds.count(accountId))
		cacheKeyIds[accountId] = uint160(0);
	else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[accountId] = uint160(0);
		}
	}
	return true;
}
bool CAccountViewCache::GetAccount(const vector<unsigned char> &accountId, CAccount &account) {
	if(cacheKeyIds.count(accountId)) {
		CKeyID keyId(cacheKeyIds[accountId]);
		if(keyId != uint160(0)) {
			if(cacheAccounts.count(keyId)){
				account = cacheAccounts[keyId];
				if(account.keyID != uint160(0)) {  // 判断此帐户是否被删除了
					return true;
				}else {
					return false;   //已删除返回false
				}
			}else {
				return pBase->GetAccount(keyId, account); //缓存map中没有，从上级存取
			}
		}
		else {
			return false;  //accountId已删除说明账户信息也已删除
		}
	}else {
		CKeyID keyId;
		if(pBase->GetKeyId(accountId, keyId)) {
			cacheKeyIds[accountId] = keyId;
			if (cacheAccounts.count(keyId) > 0 ) {
				account = cacheAccounts[keyId];
				if (account.keyID != uint160(0)) { // 判断此帐户是否被删除了
					return true;
				} else {
					return false;   //已删除返回false
				}
			}
			bool ret = pBase->GetAccount(keyId, account);
			if(ret) {
				cacheAccounts[keyId] = account;
				return true;
			}
		}
	}
	return false;
}
bool CAccountViewCache::SaveAccountInfo(const CRegID &regid, const CKeyID &keyId, const CAccount &account) {
	cacheKeyIds[regid.GetVec6()] = keyId;
	cacheAccounts[keyId] = account;
	cacheAccounts[keyId].accountOperLog.SetNULL();
	return true;
}
bool CAccountViewCache::GetAccount(const CUserID &userId, CAccount &account) {
	bool ret = false;
	if (userId.type() == typeid(CRegID)) {
		ret = GetAccount(boost::get<CRegID>(userId).GetVec6(), account);
		if(ret) assert(boost::get<CRegID>(userId) == account.regID);
	} else if (userId.type() == typeid(CKeyID)) {
		ret = GetAccount(boost::get<CKeyID>(userId), account);
		if(ret) assert(boost::get<CKeyID>(userId) == account.keyID);
	} else if (userId.type() == typeid(CPubKey)) {
		ret = GetAccount(boost::get<CPubKey>(userId).GetKeyID(), account);
		if(ret) assert((boost::get<CPubKey>(userId)).GetKeyID() == account.keyID);
	} else {
		assert(0);
	}
	return ret;
}
bool CAccountViewCache::GetKeyId(const CUserID &userId, CKeyID &keyId) {
	if (userId.type() == typeid(CRegID)) {
		return GetKeyId(boost::get<CRegID>(userId).GetVec6(), keyId);
	} else if (userId.type() == typeid(CPubKey)) {
		keyId = boost::get<CPubKey>(userId).GetKeyID();
		return true;
	} else if (userId.type() == typeid(CKeyID)) {
		keyId = boost::get<CKeyID>(userId);
		return true;
	} else
	{
		assert(0);
	}
	return false;
}
bool CAccountViewCache::SetAccount(const CUserID &userId, const CAccount &account) {
	if (userId.type() == typeid(CRegID)) {
		return SetAccount(boost::get<CRegID>(userId).GetVec6(), account);
	} else if (userId.type() == typeid(CKeyID)) {
		return SetAccount(boost::get<CKeyID>(userId), account);
	} else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::SetKeyId(const CUserID &userId, const CKeyID &keyId) {
	if (userId.type() == typeid(CRegID)) {
		return SetKeyId(boost::get<CRegID>(userId).GetVec6(), keyId);
	} else {
		assert(0);
	}

	return false;

}
bool CAccountViewCache::EraseAccount(const CUserID &userId) {
	if (userId.type() == typeid(CKeyID)) {
		return EraseAccount(boost::get<CKeyID>(userId));
	} else if(userId.type() == typeid(CPubKey)) {
		return EraseAccount(boost::get<CPubKey>(userId).GetKeyID());
	}
	else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::HaveAccount(const CUserID &userId) {
	if (userId.type() == typeid(CKeyID)) {
		return HaveAccount(boost::get<CKeyID>(userId));
	} else {
		assert(0);
	}
	return false;
}
bool CAccountViewCache::EraseId(const CUserID &userId) {
	if (userId.type() == typeid(CRegID)) {
		return EraseKeyId(boost::get<CRegID>(userId).GetVec6());
	} else {
		assert(0);
	}
	return false;
}

bool CAccountViewCache::Flush(){
	 bool fOk = pBase->BatchWrite(cacheAccounts, cacheKeyIds, hashBlock);
	 if (fOk) {
		cacheAccounts.clear();
		cacheKeyIds.clear();
	 }
	 return fOk;
}

bool CAccountViewCache::GetRegId(const CUserID& userId, CRegID& regId) const{

	CAccountViewCache tempView(*this);
	CAccount account;
	if(userId.type() == typeid(CRegID)) {
		regId = boost::get<CRegID>(userId);
		return true;
	}
	if(tempView.GetAccount(userId,account))
	{
		regId =  account.regID;
		return !regId.IsEmpty();
	}
	return false;
}

int64_t CAccountViewCache::GetRawBalance(const CUserID& userId,int curhigh) const {
	CAccountViewCache tempvew(*this);
	CAccount account;
	if(tempvew.GetAccount(userId,account))
	{
		return  account.GetRawBalance(curhigh);
	}
	return 0;
}

unsigned int CAccountViewCache::GetCacheSize(){
	return ::GetSerializeSize(cacheAccounts, SER_DISK, CLIENT_VERSION) + ::GetSerializeSize(cacheKeyIds, SER_DISK, CLIENT_VERSION);
}

Object CAccountViewCache::ToJosnObj() const {
	Object obj;
	Array arrayObj;
	obj.push_back(Pair("hashBlock", hashBlock.ToString()));
	arrayObj.push_back(pBase->ToJosnObj('a'));
	arrayObj.push_back(pBase->ToJosnObj('k'));
	obj.push_back(Pair("cacheView", arrayObj));
//	Array arrayObj;
//	for (auto& item : cacheAccounts) {
//		Object obj;
//		obj.push_back(Pair("keyID", item.first.ToString()));
//		obj.push_back(Pair("account", item.second.ToString()));
//		arrayObj.push_back(obj);
//	}
//	obj.push_back(Pair("cacheAccounts", arrayObj));
//
//	for (auto& item : cacheKeyIds) {
//		Object obj;
//		obj.push_back(Pair("accountID", HexStr(item.first)));
//		obj.push_back(Pair("keyID", item.second.ToString()));
//		arrayObj.push_back(obj);
//	}
//
//	obj.push_back(Pair("cacheKeyIds", arrayObj));
	return obj;
}

bool CScriptDBView::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {	return false;}
bool CScriptDBView::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {return false;}
bool CScriptDBView::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {return false;}
bool CScriptDBView::EraseKey(const vector<unsigned char> &vKey) {return false;}
bool CScriptDBView::HaveData(const vector<unsigned char> &vKey) {return false;}
bool CScriptDBView::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {return false;}
bool CScriptDBView::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId, const int &nIndex,
		vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight,
		set<CScriptDBOperLog> &setOperLog) {
	return false;
}
bool CScriptDBView::GetAccountAuthor(const CRegID & acctRegId, vector<pair<CRegID, CAuthorizate> > & vAuthorizate) { return false;}
Object CScriptDBView:: ToJosnObj(string Prefix){
	Object obj;
	return obj;
}
CScriptDBViewBacked::CScriptDBViewBacked(CScriptDBView &dataBaseView) {pBase = &dataBaseView;}
bool CScriptDBViewBacked::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {return pBase->GetData(vKey, vValue);}
bool CScriptDBViewBacked::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {return pBase->SetData(vKey, vValue);}
bool CScriptDBViewBacked::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapDatas) {return pBase->BatchWrite(mapDatas);}
bool CScriptDBViewBacked::EraseKey(const vector<unsigned char> &vKey) {return pBase->EraseKey(vKey);}
bool CScriptDBViewBacked::HaveData(const vector<unsigned char> &vKey) {return pBase->HaveData(vKey);}
bool CScriptDBViewBacked::GetScript(const int &nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {return pBase->GetScript(nIndex, vScriptId, vValue);}
bool CScriptDBViewBacked::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId,
		const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight,
		set<CScriptDBOperLog> &setOperLog) {
	return pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndex, vScriptKey, vScriptData, nHeight, setOperLog);
}
bool CScriptDBViewBacked::GetAccountAuthor(const CRegID & acctRegId, vector<pair<CRegID, CAuthorizate> > & vAuthorizate) {
	return pBase->GetAccountAuthor(acctRegId, vAuthorizate);
}

CScriptDBViewCache::CScriptDBViewCache(CScriptDBView &base, bool fDummy) : CScriptDBViewBacked(base) {
	mapDatas.clear();
}
bool CScriptDBViewCache::GetData(const vector<unsigned char> &vKey, vector<unsigned char> &vValue) {
	if (mapDatas.count(vKey) > 0) {
		if (!mapDatas[vKey].empty()) {
			vValue = mapDatas[vKey];
			return true;
		} else {
			return false;
		}
	}
	if (!pBase->GetData(vKey, vValue)) {
		return false;
	}
	mapDatas[vKey] = vValue;
	return true;
}
bool CScriptDBViewCache::SetData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {
	mapDatas[vKey] = vValue;
	return true;
}
bool CScriptDBViewCache::UndoScriptData(const vector<unsigned char> &vKey, const vector<unsigned char> &vValue) {
	vector<unsigned char> vPrefix(vKey.begin(), vKey.begin() + 4);
	vector<unsigned char> vScriptDataPrefix = { 'd', 'a', 't', 'a' };
	if (vPrefix == vScriptDataPrefix) {
		assert(vKey.size() > 10);
		if (vKey.size() < 10) {
			return ERRORMSG("UndoScriptData(): vKey=%s error!\n", HexStr(vKey));
		}
		vector<unsigned char> vScriptCountKey = { 's', 'd', 'n', 'u', 'm' };
		vector<unsigned char> vScriptId(vKey.begin() + 4, vKey.begin() + 10);
		vector<unsigned char> vOldValue;
		if (mapDatas.count(vKey)) {
			vOldValue = mapDatas[vKey];
		} else {
			GetData(vKey, vOldValue);
		}
		vScriptCountKey.insert(vScriptCountKey.end(), vScriptId.begin(), vScriptId.end());
		CDataStream ds(SER_DISK, CLIENT_VERSION);

		int nCount(0);
		if (vValue.empty()) {   //key所对应的值由非空设置为空，计数减1
			if (!vOldValue.empty()) {
				if (!GetScriptDataCount(vScriptId, nCount))
					return false;
				--nCount;
				if (!SetScriptDataCount(vScriptId, nCount))
					return false;
			}
		} else {    //key所对应的值由空设置为非空，计数加1
			if (vOldValue.empty()) {
				GetScriptDataCount(vScriptId, nCount);
				++nCount;
				if (!SetScriptDataCount(vScriptId, nCount))
					return false;
			}
		}
	}
	mapDatas[vKey] = vValue;
	return true;
}
bool CScriptDBViewCache::BatchWrite(const map<vector<unsigned char>, vector<unsigned char> > &mapData) {
	for (auto &items : mapData) {
		mapDatas[items.first] = items.second;
	}
	return true;
}
bool CScriptDBViewCache::EraseKey(const vector<unsigned char> &vKey) {
	if (mapDatas.count(vKey) > 0) {
		mapDatas[vKey].clear();
	} else {
		vector<unsigned char> vValue;
		if (pBase->GetData(vKey, vValue)) {
			vValue.clear();
			mapDatas[vKey] = vValue;
		}
		else {
			return false;
		}
	}
	return true;
}
bool CScriptDBViewCache::HaveData(const vector<unsigned char> &vKey) {
	if (mapDatas.count(vKey) > 0) {
		if(!mapDatas[vKey].empty())
			return true;
		else
			return false;
	}
	return pBase->HaveData(vKey);
}
bool CScriptDBViewCache::GetScript(const int nIndex, vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {
	if (0 == nIndex) {
		vector<unsigned char> scriptKey = {'d','e','f'};
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataKey.clear();
		vDataValue.clear();
		for (auto &item : mapDatas) {   //遍历本级缓存数据，找出合法的最小的key值
			vector<unsigned char> vTemp(item.first.begin(), item.first.begin() + 3);
			if (scriptKey == vTemp) {
				if (item.second.empty()) {
					continue;
				}
				vDataKey = item.first;
				vDataValue = item.second;
				break;
			}
		}
		if (!pBase->GetScript(nIndex, vScriptId, vValue)) { //上级没有获取符合条件的key值
			if (vDataKey.empty())
				return false;
			else {//返回本级缓存的查询结果
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		} else { //上级获取到符合条件的key值
			if (vDataKey.empty()) {  //缓存中没有符合条件的key，直接返回上级的查询结果
				return true;
			}
			vector<unsigned char> dataKeyTemp = {'d', 'e', 'f'};
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptId.begin(), vScriptId.end()); //上级得到的key值
			if (dataKeyTemp < vDataKey) {  //若上级查询的key小于本级缓存的key,且此key在缓存中没有，则直接返回数据库中查询的结果
				if (mapDatas.count(dataKeyTemp) == 0)
					return true;
				else {
					mapDatas[dataKeyTemp].clear();  //在缓存中dataKeyTemp已经被删除过了，重新将此key对应的value清除
					return GetScript(nIndex, vScriptId, vValue); //重新从数据库中获取下一条数据
				}
			} else {  //若上级查询的key大于等于本级缓存的key,返回本级的数据
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		}
	} else if (1 == nIndex) {
		vector<unsigned char> vKey = { 'd','e','f' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vector<unsigned char> vPreKey(vKey);
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
 		vDataKey.clear();
 		vDataValue.clear();
		vector<unsigned char> vKeyTemp={'d','e','f'};
		while (iterFindKey != mapDatas.end()) {
			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + 3);
			if (vKeyTemp == vTemp) {
				if (iterFindKey->second.empty()){
					++iterFindKey;
					continue;
				}
				else {
					vDataKey = iterFindKey->first;
					vDataValue = iterFindKey->second;
					break;
				}
			}else {
				++iterFindKey;
			}
		}
		if (!pBase->GetScript(nIndex, vScriptId, vValue)) { //从BASE获取指定键值之后的下一个值
			if (vDataKey.empty())
				return false;
			else {
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		} else {
			if (vDataKey.empty())    //缓存中没有符合条件的key，直接返回上级的查询结果
				return true;
			vector<unsigned char> dataKeyTemp = {'d', 'e', 'f'};
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptId.begin(), vScriptId.end()); //上级得到的key值
			if (dataKeyTemp < vDataKey) {
				if (mapDatas.count(dataKeyTemp) == 0)
						return true;
				else {
					mapDatas[dataKeyTemp].clear();  //在缓存中dataKeyTemp已经被删除过了，重新将此key对应的value清除
					return GetScript(nIndex, vScriptId, vValue); //重新从数据库中获取下一条数据
				}
			} else { //若上级查询的key大于等于本级缓存的key,返回本级的数据
				vScriptId.clear();
				vValue.clear();
				vScriptId.assign(vDataKey.begin()+3, vDataKey.end());
				vValue = vDataValue;
				return true;
			}
		}

	} else {
		assert(0);
	}
	return true;
}
bool CScriptDBViewCache::SetScript(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vValue) {
	vector<unsigned char> scriptKey = {'d','e','f'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());

	if (!HaveScript(vScriptId)) {
		int nCount(0);
		GetScriptCount(nCount);
		++nCount;
		if (!SetScriptCount(nCount))
			return false;
	}
	return SetData(scriptKey, vValue);
}
bool CScriptDBViewCache::Flush() {
	bool ok = pBase->BatchWrite(mapDatas);
	if(ok) {
		mapDatas.clear();
	}
	return ok;
}
unsigned int CScriptDBViewCache::GetCacheSize() {
	return ::GetSerializeSize(mapDatas, SER_DISK, CLIENT_VERSION);
}

bool CScriptDBViewCache::GetScript(const vector<unsigned char> &vScriptId, vector<unsigned char> &vValue) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };

	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	return GetData(scriptKey, vValue);
}
bool CScriptDBViewCache::GetScript(const CRegID &scriptId, vector<unsigned char> &vValue) {
	return GetScript(scriptId.GetVec6(), vValue);
}

bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId,
		const vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight,
		CScriptDBOperLog &operLog) {
//	assert(vScriptKey.size() == 8);
	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };

	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), vScriptKey.begin(), vScriptKey.end());
	vector<unsigned char> vValue;
	if (!GetData(vKey, vValue))
		return false;
	if(vValue.empty())
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> vScriptData;
	if (nHeight <= nCurBlockHeight) {
		EraseScriptData(vScriptId, vScriptKey, operLog);
		return false;
	}
	return true;
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const vector<unsigned char> &vScriptId,
		const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData, int &nHeight,
		set<CScriptDBOperLog> &setOperLog) {
	if(0 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vKey.push_back('_');
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataKey.clear();
		vDataValue.clear();
		for (auto &item : mapDatas) {   //遍历本级缓存数据，找出合法的最小的key值
			vector<unsigned char> vTemp(item.first.begin(),item.first.begin()+vScriptId.size()+5);
			if(vKey == vTemp) {
				if(item.second.empty()) {
					continue;
				}
				vDataKey = item.first;
				vDataValue = item.second;
				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				if(nHeight <= nCurBlockHeight) { //若找到的key对应的数据保存时间已经超时，则需要删除该数据项，继续找下一个符合条件的key
					CScriptDBOperLog operLog(vDataKey, vDataValue);
					vDataKey.clear();
					vDataValue.clear();
					setOperLog.insert(operLog);
					continue;
				}
				break;
			}
		}
		bool bUpLevelRet(false);
		unsigned long llCount(0);
		int nIndexTemp = nIndex;
		while((bUpLevelRet = pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndexTemp, vScriptKey, vScriptData, nHeight, setOperLog))) {
//			LogPrint("INFO", "nCurBlockHeight:%d this addr:0x%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//					nCurBlockHeight, this, nIndexTemp, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
			nIndexTemp = 1;
			set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
			for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
				if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
					setOperLog.erase(iterOperLog++);
				}else {
					++iterOperLog;
				}
			}
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
//			LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//			if(mapDatas.count(dataKeyTemp) > 0) {//本级缓存包含上级查询结果的key
//				if(dataKeyTemp != vDataKey) {  //本级和上级查找key不同，说明上级获取的数据在本级已被删除
//					continue;
//				} else {
//					if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//						continue;
//					}
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			} else {//本级缓存不包含上级查询结果key
//				if (dataKeyTemp < vDataKey) { //上级获取key值小
//					return true;      //返回上级的查询结果
//				} else {              //本级查询结果Key值小，返回本级查询结果
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			}
			if (vDataKey.empty()) {   //缓存中没有符合条件的key，直接返回上级的查询结果
				if(mapDatas.count(dataKeyTemp) <= 0)
					return true;
				else {
//					LogPrint("INFO", "local level contains dataKeyTemp,but the value is empty,need redo getscriptdata()\n");
					continue;			 //重新从数据库中获取下一条数据
				}
			}
			else {
				if (dataKeyTemp < vDataKey) {
					if(mapDatas.count(dataKeyTemp) <= 0) {
						return true;
					}
					else {
//						LogPrint("INFO", "dataKeyTemp less than vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;			 //重新从数据库中获取下一条数据
					}
				} else {
					if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;
					}
					vScriptKey.clear();
					vScriptData.clear();
					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
					ds >> nHeight;
					ds >> vScriptData;
					return true;
				}
			}
		}
		if(!bUpLevelRet) {
			set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
			for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
				if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
					setOperLog.erase(iterOperLog++);
				}else{
					++iterOperLog;
				}
			}
			if (vDataKey.empty())
				return false;
			else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				if (vDataValue.empty()) {
					return false;
				}
				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				ds >> vScriptData;
				return true;
			}
		}
	}
	else if (1 == nIndex) {
		vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
		vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
		vKey.push_back('_');
		vector<unsigned char> vPreKey(vKey);
		vPreKey.insert(vPreKey.end(), vScriptKey.begin(), vScriptKey.end());
		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
		vector<unsigned char> vDataKey;
		vector<unsigned char> vDataValue;
		vDataValue.clear();
		vDataKey.clear();
		while (iterFindKey != mapDatas.end()) {
			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vScriptId.size() + 5);
			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + vScriptId.size() + 5);
			if (vKeyTemp == vTemp) {
				if (iterFindKey->second.empty()) {
					++iterFindKey;
					continue;
				} else {
					vDataKey = iterFindKey->first;
					vDataValue = iterFindKey->second;
					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
					ds >> nHeight;
					if (nHeight <= nCurBlockHeight) { //若找到的key对应的数据保存时间已经超时，则需要删除该数据项，继续找下一个符合条件的key
						CScriptDBOperLog operLog(vDataKey, iterFindKey->second);
						vDataKey.clear();
						vDataValue.clear();
						setOperLog.insert(operLog);
						++iterFindKey;
						continue;
					}
					break;
				}
			} else {
				++iterFindKey;
			}
		}
		bool bUpLevelRet(false);
		unsigned long llCount(0);
		while((bUpLevelRet=pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndex, vScriptKey, vScriptData, nHeight, setOperLog))) {
//			LogPrint("INFO", "nCurBlockHeight:%d this addr:0x%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//					nCurBlockHeight, this, nIndex, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
			set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
			for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
				if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
					setOperLog.erase(iterOperLog++);
				}else {
					++iterOperLog;
				}
			}
			vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
			dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
//			LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//			if(mapDatas.count(dataKeyTemp) > 0) {//本级缓存包含上级查询结果的key
//				if(dataKeyTemp != vDataKey) {  //本级和上级查找key不同，说明上级获取的数据在本级已被删除
//					continue;
//				} else {
//					if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//						continue;
//					}
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			} else {//本级缓存不包含上级查询结果key
//				if (dataKeyTemp < vDataKey) { //上级获取key值小
//					return true;      //返回上级的查询结果
//				} else {              //本级查询结果Key值小，返回本级查询结果
//					vScriptKey.clear();
//					vScriptData.clear();
//					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					ds >> vScriptData;
//					return true;
//				}
//			}
			if (vDataKey.empty()) {   //缓存中没有符合条件的key，直接返回上级的查询结果
				if(mapDatas.count(dataKeyTemp) <= 0)
					return true;
				else {
//					LogPrint("INFO", "local level contains dataKeyTemp,but the value is empty,need redo getscriptdata()\n");
					continue;			 //重新从数据库中获取下一条数据
				}
			}
			else {
				if (dataKeyTemp < vDataKey) {
					if(mapDatas.count(dataKeyTemp) == 0)
						return true;
					else {
//						LogPrint("INFO", "dataKeyTemp less than vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;			//在缓存中dataKeyTemp已经被删除过了，重新从数据库中获取下一条数据
					}
				} else {
					if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//						LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
						continue;
					}
					vScriptKey.clear();
					vScriptData.clear();
					vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
					ds >> nHeight;
					ds >> vScriptData;
					return true;
				}
			}
		}
		if(!bUpLevelRet) {
			set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
			for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
				if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
					setOperLog.erase(iterOperLog++);
				}else{
					++iterOperLog;
				}
			}
			if (vDataKey.empty())
				return false;
			else {
				vScriptKey.clear();
				vScriptData.clear();
				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
				if (vDataValue.empty()) {
					return false;
				}
				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
				ds >> nHeight;
				ds >> vScriptData;
				return true;
			}
		}
	}
	else {
		assert(0);
	}
//	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
//	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
//	vKey.push_back('_');
//	vector<unsigned char> vDataKey;
//	vector<unsigned char> vDataValue;
//	if(0 == nIndex) {
//		vDataKey.clear();
//		vDataValue.clear();
//		for (auto &item : mapDatas) {   //遍历本级缓存数据，找出合法的最小的key值
//			vector<unsigned char> vTemp(item.first.begin(),item.first.begin()+vScriptId.size()+5);
//			if(vKey == vTemp) {
//				if(item.second.empty()) {
//					continue;
//				}
//				vDataKey = item.first;
//				vDataValue = item.second;
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				if(nHeight <= nCurBlockHeight) { //若找到的key对应的数据保存时间已经超时，则需要删除该数据项，继续找下一个符合条件的key
//					CScriptDBOperLog operLog(vDataKey, vDataValue);
//					vDataKey.clear();
//					vDataValue.clear();
//					setOperLog.insert(operLog);
//					continue;
//				}
//				break;
//			}
//		}
//	}
//	else if (1 == nIndex) {
//		vector<unsigned char> vPreKey(vKey);
//		vPreKey.insert(vPreKey.end(), vScriptKey.begin(), vScriptKey.end());
//		map<vector<unsigned char>, vector<unsigned char> >::iterator iterFindKey = mapDatas.upper_bound(vPreKey);
//		vDataValue.clear();
//		vDataKey.clear();
//		while (iterFindKey != mapDatas.end()) {
//			vector<unsigned char> vKeyTemp(vKey.begin(), vKey.begin() + vScriptId.size() + 5);
//			vector<unsigned char> vTemp(iterFindKey->first.begin(), iterFindKey->first.begin() + vScriptId.size() + 5);
//			if (vKeyTemp == vTemp) {
//				if (iterFindKey->second.empty()) {
//					++iterFindKey;
//					continue;
//				} else {
//					vDataKey = iterFindKey->first;
//					vDataValue = iterFindKey->second;
//					CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//					ds >> nHeight;
//					if (nHeight <= nCurBlockHeight) { //若找到的key对应的数据保存时间已经超时，则需要删除该数据项，继续找下一个符合条件的key
//						CScriptDBOperLog operLog(vDataKey, iterFindKey->second);
//						vDataKey.clear();
//						vDataValue.clear();
//						setOperLog.insert(operLog);
//						++iterFindKey;
//						continue;
//					}
//					break;
//				}
//			} else {
//				++iterFindKey;
//			}
//		}
//	}
//	else {
//		assert(0);
//	}
//	bool bUpLevelRet(false);
//	unsigned long llCount(0);
//	int nIndexTemp = nIndex;
//	while((bUpLevelRet = pBase->GetScriptData(nCurBlockHeight, vScriptId, nIndexTemp, vScriptKey, vScriptData, nHeight, setOperLog))) {
//		LogPrint("INFO", "nCurBlockHeight:%d this addr:%x, nIndex:%d, count:%lld\n ScriptKey:%s\n nHeight:%d\n ScriptData:%s\n vDataKey:%s\n vDataValue:%s\n",
//				nCurBlockHeight, this, nIndexTemp, ++llCount, HexStr(vScriptKey), nHeight, HexStr(vScriptData), HexStr(vDataKey), HexStr(vDataValue));
//		nIndexTemp = 1;
//		for(auto &itemKey : mapDatas) {
//			vector<unsigned char> vKeyTemp(itemKey.first.begin(), itemKey.first.begin() + vKey.size());
//			if(vKeyTemp == vKey) {
//				LogPrint("INFO", "vKey:%s\n vValue:%s\n", HexStr(itemKey.first), HexStr(itemKey.second));
//			}
//		}
//		set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
//		for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
//			if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
//				LogPrint("INFO", "DeleteData key:%s\n", HexStr(iterOperLog->vKey));
//				setOperLog.erase(iterOperLog++);
//			}else {
//				++iterOperLog;
//			}
//		}
//		vector<unsigned char> dataKeyTemp(vKey.begin(), vKey.end());
//		dataKeyTemp.insert(dataKeyTemp.end(), vScriptKey.begin(), vScriptKey.end());
//		LogPrint("INFO", "dataKeyTemp:%s\n vDataKey:%s\n", HexStr(dataKeyTemp), HexStr(vDataKey));
//		if(mapDatas.count(dataKeyTemp) > 0) {//本级缓存包含上级查询结果的key
//			if(dataKeyTemp != vDataKey) {  //本级和上级查找key不同，说明上级获取的数据在本级已被删除
//				LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//				continue;
//			} else {
//				if(vDataValue.empty()) { //本级和上级数据key相同,且本级数据已经删除，重新从上级获取下一条数据
//					LogPrint("INFO", "dataKeyTemp equal vDataKey and vDataValue empty redo getscriptdata()\n");
//					continue;
//				}
//				vScriptKey.clear();
//				vScriptData.clear();
//				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				ds >> vScriptData;
//				return true;
//			}
//		} else {//本级缓存不包含上级查询结果key
//			if (dataKeyTemp < vDataKey) { //上级获取key值小
//				return true;      //返回上级的查询结果
//			} else {              //本级查询结果Key值小，返回本级查询结果
//				vScriptKey.clear();
//				vScriptData.clear();
//				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				ds >> vScriptData;
//				return true;
//			}
//		}
//		if(!bUpLevelRet) {
//			set<CScriptDBOperLog>::iterator iterOperLog = setOperLog.begin();
//			for (; iterOperLog != setOperLog.end();) { //防止由于没有flush cache，对数据库中超时的脚本数据项，在cache中多次删除，引起删除失败
//				if (mapDatas.count(iterOperLog->vKey) > 0 && mapDatas[iterOperLog->vKey].empty()) {
//					LogPrint("INFO", "DeleteData key:%s\n", HexStr(iterOperLog->vKey));
//					setOperLog.erase(iterOperLog++);
//				}else{
//					++iterOperLog;
//				}
//			}
//			if (vDataKey.empty())
//				return false;
//			else {
//				vScriptKey.clear();
//				vScriptData.clear();
//				vScriptKey.insert(vScriptKey.end(), vDataKey.begin() + 11, vDataKey.end());
//				if (vDataValue.empty()) {
//					return false;
//				}
//				CDataStream ds(vDataValue, SER_DISK, CLIENT_VERSION);
//				ds >> nHeight;
//				ds >> vScriptData;
//				return true;
//			}
//		}
//	}
	return true;
}
bool CScriptDBViewCache::SetScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey,
		const vector<unsigned char> &vScriptData, const int nHeight, CScriptDBOperLog &operLog) {
	vector<unsigned char> vKey = { 'd', 'a', 't', 'a' };
	vKey.insert(vKey.end(), vScriptId.begin(), vScriptId.end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), vScriptKey.begin(), vScriptKey.end());
  //  LogPrint("vm","add data:%s",HexStr(vScriptKey).c_str());
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << nHeight;
	ds << vScriptData;
	vector<unsigned char> vValue(ds.begin(), ds.end());
	if (!HaveScriptData(vScriptId, vScriptKey)) {
		int nCount(0);
		GetScriptDataCount(vScriptId, nCount);
		++nCount;
		if (!SetScriptDataCount(vScriptId, nCount))
			return false;
	}
	vector<unsigned char> oldValue;
	oldValue.clear();
	GetData(vKey, oldValue);
	operLog = CScriptDBOperLog(vKey, oldValue);

	return SetData(vKey, vValue);
}

bool CScriptDBViewCache::HaveScript(const vector<unsigned char> &vScriptId) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	return HaveData(scriptKey);
}
bool CScriptDBViewCache::GetScriptCount(int &nCount) {
	vector<unsigned char> scriptKey = { 's', 'n', 'u','m'};
	vector<unsigned char> vValue;
	if (!GetData(scriptKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> nCount;
	return true;
}
bool CScriptDBViewCache::SetScriptCount(const int nCount) {
	vector<unsigned char> scriptKey = { 's', 'n', 'u','m'};
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << nCount;
	vector<unsigned char> vValue(ds.begin(), ds.end());
	if(!SetData(scriptKey, vValue))
		return false;
	return true;
}
bool CScriptDBViewCache::EraseScript(const vector<unsigned char> &vScriptId) {
	vector<unsigned char> scriptKey = { 'd', 'e', 'f' };
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	if (HaveScript(vScriptId)) {
		int nCount(0);
		if (!GetScriptCount(nCount))
			return false;
		--nCount;
		if (!SetScriptCount(nCount))
			return false;
	}
	return EraseKey(scriptKey);
}
bool CScriptDBViewCache::GetScriptDataCount(const vector<unsigned char> &vScriptId, int &nCount) {
	vector<unsigned char> scriptKey = { 's', 'd', 'n', 'u','m'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	vector<unsigned char> vValue;
	if(!GetData(scriptKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> nCount;
	return true;
}
bool CScriptDBViewCache::SetScriptDataCount(const vector<unsigned char> &vScriptId, int nCount) {
	vector<unsigned char> scriptKey = { 's', 'd', 'n', 'u','m'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << nCount;
	vector<unsigned char> vValue(ds.begin(), ds.end());
	if(!SetData(scriptKey, vValue))
		return false;
	return true;
}

bool CScriptDBViewCache::EraseScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog) {
	vector<unsigned char> scriptKey = { 'd', 'a', 't', 'a'};

	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	scriptKey.push_back('_');
	scriptKey.insert(scriptKey.end(), vScriptKey.begin(), vScriptKey.end());

	if (HaveScriptData(vScriptId, vScriptKey)) {
		int nCount(0);
		if(!GetScriptDataCount(vScriptId, nCount)) {
			return false;
		}
		--nCount;
		if (!SetScriptDataCount(vScriptId, nCount)) {
			return false;
		}

	}

	vector<unsigned char> vValue;
	if(!GetData(scriptKey, vValue)) {
		return false;
	}

	operLog = CScriptDBOperLog(scriptKey, vValue);
	if(!EraseKey(scriptKey))
		return false;


	return true;
}

bool CScriptDBViewCache::EraseScriptData(const vector<unsigned char> &vKey) {
	if(vKey.size() < 12) {
		assert(0);
	}
	vector<unsigned char> vScriptId(vKey.begin()+4, vKey.begin()+10);
	vector<unsigned char> vScriptKey(vKey.begin()+11, vKey.end());
	CScriptDBOperLog operLog;
	return EraseScriptData(vScriptId, vScriptKey, operLog);
}

bool CScriptDBViewCache::HaveScriptData(const vector<unsigned char> &vScriptId, const vector<unsigned char > &vScriptKey) {
	vector<unsigned char> scriptKey = { 'd', 'a', 't', 'a'};
	scriptKey.insert(scriptKey.end(), vScriptId.begin(), vScriptId.end());
	scriptKey.push_back('_');
	scriptKey.insert(scriptKey.end(), vScriptKey.begin(), vScriptKey.end());
	return HaveData(scriptKey);
}


bool CScriptDBViewCache::GetScript(const int nIndex, CRegID &scriptId, vector<unsigned char> &vValue) {
	vector<unsigned char> tem;
	if (nIndex != 0) {
		tem = scriptId.GetVec6();
	}
	if (GetScript(nIndex, tem, vValue)) {
		scriptId.SetRegID(tem);
		return true;
	}

	return false;
}
bool CScriptDBViewCache::SetScript(const CRegID &scriptId, const vector<unsigned char> &vValue) {
	return SetScript(scriptId.GetVec6(), vValue);
}
bool CScriptDBViewCache::HaveScript(const CRegID &scriptId) {
	return HaveScript(scriptId.GetVec6());
}
bool CScriptDBViewCache::EraseScript(const CRegID &scriptId) {
	return EraseScript(scriptId.GetVec6());
}
bool CScriptDBViewCache::GetScriptDataCount(const CRegID &scriptId, int &nCount) {
	return  GetScriptDataCount(scriptId.GetVec6(), nCount);
}
bool CScriptDBViewCache::EraseScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey, CScriptDBOperLog &operLog) {
	bool  temp = EraseScriptData(scriptId.GetVec6(), vScriptKey, operLog);
	return temp;
}
bool CScriptDBViewCache::HaveScriptData(const CRegID &scriptId, const vector<unsigned char > &vScriptKey) {
	return HaveScriptData(scriptId.GetVec6(), vScriptKey);
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
			vector<unsigned char> &vScriptData, int &nHeight, CScriptDBOperLog &operLog) {
	return GetScriptData(nCurBlockHeight, scriptId.GetVec6() , vScriptKey, vScriptData, nHeight, operLog);
}
bool CScriptDBViewCache::GetScriptData(const int nCurBlockHeight, const CRegID &scriptId, const int &nIndex, vector<unsigned char> &vScriptKey, vector<unsigned char> &vScriptData,
		int &nHeight, set<CScriptDBOperLog> &setOperLog) {
	bool bRet = GetScriptData(nCurBlockHeight, scriptId.GetVec6(), nIndex, vScriptKey, vScriptData, nHeight, setOperLog);
	if(!setOperLog.empty()) {    //删除已经超时的数据项
		for (auto &item : setOperLog) {
			if(!EraseScriptData(item.vKey)) {
				return ERRORMSG("GetScriptData() delete timeout script data item of super level db error");
			}
		}
	}
	return bRet;
}
bool CScriptDBViewCache::SetScriptData(const CRegID &scriptId, const vector<unsigned char> &vScriptKey,
			const vector<unsigned char> &vScriptData, const int nHeight, CScriptDBOperLog &operLog) {
	bool  temp =SetScriptData(scriptId.GetVec6(), vScriptKey, vScriptData, nHeight, operLog);
	return temp;
}
bool CScriptDBViewCache::SetTxRelAccout(const uint256 &txHash, const set<CKeyID> &relAccount) {
	vector<unsigned char> vKey = {'t','x'};
	vector<unsigned char> vValue;
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << txHash;
	vKey.insert(vKey.end(), ds.begin(), ds.end());
	ds.clear();
	ds << relAccount;
	vValue.assign(ds.begin(), ds.end());
	return SetData(vKey, vValue);
}
bool CScriptDBViewCache::GetTxRelAccount(const uint256 &txHash, set<CKeyID> &relAccount) {
	vector<unsigned char> vKey = {'t','x'};
	vector<unsigned char> vValue;
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << txHash;
	vKey.insert(vKey.end(), ds.begin(), ds.end());
	if(!GetData(vKey, vValue))
		return false;
	ds.clear();
	vector<char> temp;
	temp.assign(vValue.begin(), vValue.end());
	ds.insert(ds.end(), temp.begin(), temp.end());
	ds >> relAccount;
	return true;
}
Object CScriptDBViewCache::ToJosnObj() const {
	Object obj;
	Array arrayObj;
//	for (auto& item : mapDatas) {
//		Object obj;
//		obj.push_back(Pair("key", HexStr(item.first)));
//		obj.push_back(Pair("value", HexStr(item.second)));
//		arrayObj.push_back(obj);
//	}
//	obj.push_back(Pair("mapDatas", arrayObj));
	arrayObj.push_back(pBase->ToJosnObj("def"));
	arrayObj.push_back(pBase->ToJosnObj("data"));
	arrayObj.push_back(pBase->ToJosnObj("author"));
	obj.push_back(Pair("mapDatas", arrayObj));
	return obj;
}


bool CScriptDBViewCache::GetAuthorizate(const CRegID &acctRegId, const CRegID &scriptId, CAuthorizate &authorizate) {
	vector<unsigned char> vKey = {'a','u','t','h','o','r'};
	vector<unsigned char> vValue;
	vKey.insert(vKey.end(), acctRegId.GetVec6().begin(), acctRegId.GetVec6().end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), scriptId.GetVec6().begin(), scriptId.GetVec6().end());
	if(!GetData(vKey, vValue))
		return false;
	CDataStream ds(vValue, SER_DISK, CLIENT_VERSION);
	ds >> authorizate;
	return true;
}
bool CScriptDBViewCache::SetAuthorizate(const CRegID &acctRegId, const CRegID &scriptId, const CAuthorizate &authorizate,  CScriptDBOperLog &operLog) {
	vector<unsigned char> vKey = {'a','u','t','h','o','r'};
	vector<unsigned char> vValue;
	vKey.insert(vKey.end(), acctRegId.GetVec6().begin(), acctRegId.GetVec6().end());
	vKey.push_back('_');
	vKey.insert(vKey.end(), scriptId.GetVec6().begin(), scriptId.GetVec6().end());
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << authorizate;
	vValue.assign(ds.begin(), ds.end());
	vector<unsigned char> oldValue;
	oldValue.clear();
	GetData(vKey, oldValue);
	operLog = CScriptDBOperLog(vKey, oldValue);
	return SetData(vKey, vValue);
}

bool CScriptDBViewCache::GetAccountAuthor(const CRegID & acctRegId, vector<pair<CRegID, CAuthorizate> > & vAuthorizate) {
	return pBase->GetAccountAuthor(acctRegId, vAuthorizate);
}
uint256 CTransactionDBView::IsContainTx(const uint256 & txHash) { return std::move(uint256(0)); }
bool CTransactionDBView::IsContainBlock(const CBlock &block) { return false; }
bool CTransactionDBView::AddBlockToCache(const CBlock &block) { return false; }
bool CTransactionDBView::DeleteBlockFromCache(const CBlock &block) { return false; }
bool CTransactionDBView::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) { return false; }
bool CTransactionDBView::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHash) { return false; }



CTransactionDBViewBacked::CTransactionDBViewBacked(CTransactionDBView &transactionView) {
	pBase = &transactionView;
}
uint256 CTransactionDBViewBacked::IsContainTx(const uint256 & txHash) {
	return std::move(pBase->IsContainTx(txHash));
}
bool CTransactionDBViewBacked::IsContainBlock(const CBlock &block) {
	return pBase->IsContainBlock(block);
}

bool CTransactionDBViewBacked::AddBlockToCache(const CBlock &block) {
	return pBase->AddBlockToCache(block);
}
bool CTransactionDBViewBacked::DeleteBlockFromCache(const CBlock &block) {
	return pBase->DeleteBlockFromCache(block);
}
bool CTransactionDBViewBacked::LoadTransaction(map<uint256, vector<uint256> > &mapTxHashByBlockHash) {
	return pBase->LoadTransaction(mapTxHashByBlockHash);
}
bool CTransactionDBViewBacked::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn) {
	return pBase->BatchWrite(mapTxHashByBlockHashIn);
}

CTransactionDBCache::CTransactionDBCache(CTransactionDBView &txCacheDB, bool fDummy) : CTransactionDBViewBacked(txCacheDB){

}

bool CTransactionDBCache::IsContainBlock(const CBlock &block) {
	//(mapTxHashByBlockHash.count(block.GetHash()) > 0 && mapTxHashByBlockHash[block.GetHash()].size() > 0)
	return (IsInMap(mapTxHashByBlockHash,block.GetHash())
			|| pBase->IsContainBlock(block));
}

bool CTransactionDBCache::AddBlockToCache(const CBlock &block) {
	vector<uint256> vTxHash;
	vTxHash.clear();
	for (auto &ptx : block.vptx) {
		vTxHash.push_back(ptx->GetHash());
	}
//	if (IsContainBlock(block)) {
//		LogPrint("INFO", "the block hash:%s is in TxCache\n", block.GetHash().GetHex());
	mapTxHashByBlockHash[block.GetHash()] = vTxHash;
//	LogPrint("INFO", "CTransactionDBCache::AddBlockToCache() blockhash=%s height=%d\n", block.GetHash().GetHex(), block.nHeight);
//	} else {
//		mapTxHashByBlockHash.insert(make_pair(block.GetHash(), vTxHash));
//	}

//	LogPrint("block", "mapTxHashByBlockHash size:%d\n", mapTxHashByBlockHash.size());
//	for (auto &item : mapTxHashByBlockHash) {
//		LogPrint("INFO", "blockhash:%s\n", item.first.GetHex());
////		for (auto &txHash : item.second)
////			LogPrint("INFO", "txhash:%s\n", txHash.GetHex());
//	}
//	for(auto &item : mapTxHashCacheByPrev) {
//		LogPrint("INFO", "prehash:%s\n", item.first.GetHex());
//		for(auto &relayTx : item.second)
//			LogPrint("INFO", "relay tx hash:%s\n", relayTx.GetHex());
//	}
	return true;
}

bool CTransactionDBCache::DeleteBlockFromCache(const CBlock &block) {
//	LogPrint("INFO", "CTransactionDBCache::DeleteBlockFromCache() blockhash=%s height=%d\n", block.GetHash().GetHex(), block.nHeight);
	if (IsContainBlock(block)) {
		vector<uint256> vTxHash;
		vTxHash.clear();
		mapTxHashByBlockHash[block.GetHash()] = vTxHash;
		return true;
	} else {
		LogPrint("ERROR", "the block hash:%s isn't in TxCache\n", block.GetHash().GetHex());
		return false;
	}

	return true;
}

uint256 CTransactionDBCache::IsContainTx(const uint256 & txHash) {
	for (auto & item : mapTxHashByBlockHash) {
		vector<uint256>::iterator it = find(item.second.begin(), item.second.end(), txHash);
		if (it != item.second.end()) {
			return item.first;
		}
	}
	uint256 blockHash = pBase->IsContainTx(txHash);
	if(IsInMap(mapTxHashByBlockHash,blockHash)){//mapTxHashByBlockHash[blockHash].empty()) { // [] 运算符防止不小心加入了垃圾数据
		return std::move(blockHash);
	}
	return std::move(uint256(0));
}

map<uint256, vector<uint256> > CTransactionDBCache::GetTxHashCache(void) {
	return mapTxHashByBlockHash;
}

bool CTransactionDBCache::BatchWrite(const map<uint256, vector<uint256> > &mapTxHashByBlockHashIn) {
	for(auto & item : mapTxHashByBlockHashIn) {
		mapTxHashByBlockHash[item.first] = item.second;
	}
	return true;
}

bool CTransactionDBCache::Flush() {
	bool bRet = pBase->BatchWrite(mapTxHashByBlockHash);
//	if (bRet) {
//		mapTxHashByBlockHash.clear();
//	}
	return bRet;
}

void CTransactionDBCache::AddTxHashCache(const uint256 & blockHash, const vector<uint256> &vTxHash) {
	mapTxHashByBlockHash[blockHash] = vTxHash;
}

bool CTransactionDBCache::LoadTransaction() {
	return pBase->LoadTransaction(mapTxHashByBlockHash);
}

void CTransactionDBCache::Clear() {
	mapTxHashByBlockHash.clear();
}

bool CTransactionDBCache::IsInMap(const map<uint256, vector<uint256> >& mMap, const uint256 &hash) const {
	if (hash == 0)
		return false;
	auto te =mMap.find(hash);
	if(te != mMap.end()){
		return !te->second.empty();
	}

	return false;
}

Object CTransactionDBCache::ToJosnObj() const {
	Array deletedobjArray;
	Array InobjArray;
	for (auto& item : mapTxHashByBlockHash) {
		Object obj;
		obj.push_back(Pair("blockhash", item.first.ToString()));

		Array objTxInBlock;
		for (const auto& itemTx : item.second) {
			Object objTxHash;
			objTxHash.push_back(Pair("txhash", itemTx.ToString()));
			objTxInBlock.push_back(objTxHash);
		}
		obj.push_back(Pair("txHashs", objTxInBlock));
       if(item.second.size() > 0) {
		InobjArray.push_back(std::move(obj));
       }
       else{
    	   deletedobjArray.push_back(std::move(obj));
       }
	}
	Object temobj;
	temobj.push_back(Pair("incachblock", std::move(InobjArray)));
//	temobj.push_back(Pair("removecachblock", std::move(deletedobjArray)));
	Object retobj;
	retobj.push_back(Pair("mapTxHashByBlockHash", std::move(temobj)));
	return std::move(retobj);

}

