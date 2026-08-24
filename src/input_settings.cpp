#include "input_settings.h"
#include <LittleFS.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace {
KeyInputSetting keySettings[KEY_COUNT];
EncoderInputSetting encoderSetting;
bool fileSystemReady=false;
String keyPath(uint8_t i){return String("/key")+(i+1)+".bin";}
constexpr char ENCODER_PATH[]="/encoder.bin";
bool typeValid(OscValueType t,bool stringOk=true){return t>=OSC_TYPE_FLOAT&&t<=(stringOk?OSC_TYPE_STRING:OSC_TYPE_INT);}

using Bytes=std::vector<uint8_t>;
void addU8(Bytes& out,uint8_t value){out.push_back(value);}
void addFloat(Bytes& out,float value){const uint8_t* p=reinterpret_cast<const uint8_t*>(&value);out.insert(out.end(),p,p+sizeof(value));}
void addString(Bytes& out,const String& value){const uint16_t length=static_cast<uint16_t>(value.length());out.push_back(length&0xff);out.push_back(length>>8);out.insert(out.end(),value.c_str(),value.c_str()+length);}
bool takeU8(const Bytes& in,size_t& at,uint8_t& value){if(at>=in.size())return false;value=in[at++];return true;}
bool takeFloat(const Bytes& in,size_t& at,float& value){if(at+sizeof(value)>in.size())return false;memcpy(&value,in.data()+at,sizeof(value));at+=sizeof(value);return true;}
bool takeString(const Bytes& in,size_t& at,String& value){if(at+2>in.size())return false;const uint16_t length=in[at]|(static_cast<uint16_t>(in[at+1])<<8);at+=2;if(at+length>in.size())return false;value="";if(!value.reserve(length))return false;for(uint16_t i=0;i<length;++i)value+=static_cast<char>(in[at+i]);at+=length;return true;}
void addMessage(Bytes& out,const OscMessageSetting& message){addString(out,message.address);addString(out,message.value);addU8(out,message.type);}
bool takeMessage(const Bytes& in,size_t& at,OscMessageSetting& message){uint8_t type;return takeString(in,at,message.address)&&takeString(in,at,message.value)&&takeU8(in,at,type)&&(message.type=static_cast<OscValueType>(type),true);}
void addButton(Bytes& out,const ButtonInputSetting& button){addU8(out,button.mode);addU8(out,button.pressMessageCount);addU8(out,button.releaseMessageCount);for(uint8_t i=0;i<button.pressMessageCount;++i)addMessage(out,button.pressMessages[i]);for(uint8_t i=0;i<button.releaseMessageCount;++i)addMessage(out,button.releaseMessages[i]);addString(out,button.sequence.address);addU8(out,button.sequence.type);addFloat(out,button.sequence.start);addFloat(out,button.sequence.end);addFloat(out,button.sequence.step);}
bool takeButton(const Bytes& in,size_t& at,ButtonInputSetting& button){uint8_t mode,pressCount,releaseCount,sequenceType;if(!takeU8(in,at,mode)||!takeU8(in,at,pressCount)||!takeU8(in,at,releaseCount)||pressCount>MAX_OSC_MESSAGES||releaseCount>MAX_OSC_MESSAGES||pressCount+releaseCount>MAX_OSC_MESSAGES)return false;button.mode=static_cast<InputMode>(mode);button.pressMessageCount=pressCount;button.releaseMessageCount=releaseCount;for(uint8_t i=0;i<pressCount;++i)if(!takeMessage(in,at,button.pressMessages[i]))return false;for(uint8_t i=0;i<releaseCount;++i)if(!takeMessage(in,at,button.releaseMessages[i]))return false;if(!takeString(in,at,button.sequence.address)||!takeU8(in,at,sequenceType)||!takeFloat(in,at,button.sequence.start)||!takeFloat(in,at,button.sequence.end)||!takeFloat(in,at,button.sequence.step))return false;button.sequence.type=static_cast<OscValueType>(sequenceType);inputNormalizeSequence(button.sequence);return inputButtonSettingValid(button);}
Bytes encodeKey(const KeyInputSetting& setting){Bytes out;out.reserve(256);addU8(out,3);addString(out,setting.displayName);addButton(out,setting.button);return out;}
bool decodeKey(const Bytes& in,KeyInputSetting& setting){size_t at=0;uint8_t version;if(!takeU8(in,at,version)||version!=3||!takeString(in,at,setting.displayName)||!takeButton(in,at,setting.button)||at!=in.size())return false;return !setting.displayName.isEmpty()&&setting.displayName.length()<=64;}
Bytes encodeEncoder(const EncoderInputSetting& setting){Bytes out;out.reserve(320);addU8(out,3);addString(out,setting.displayName);addString(out,setting.rotationAddress);addU8(out,setting.sendIncrement?1:0);addFloat(out,setting.absoluteInputMin);addFloat(out,setting.absoluteInputMax);addFloat(out,setting.incrementScale);addFloat(out,setting.outputMin);addFloat(out,setting.outputMax);addU8(out,setting.outputType);addButton(out,setting.click);return out;}
bool decodeEncoder(const Bytes& in,EncoderInputSetting& setting){size_t at=0;uint8_t version,increment,type;if(!takeU8(in,at,version)||version!=3||!takeString(in,at,setting.displayName)||!takeString(in,at,setting.rotationAddress)||!takeU8(in,at,increment)||!takeFloat(in,at,setting.absoluteInputMin)||!takeFloat(in,at,setting.absoluteInputMax)||!takeFloat(in,at,setting.incrementScale)||!takeFloat(in,at,setting.outputMin)||!takeFloat(in,at,setting.outputMax)||!takeU8(in,at,type)||!takeButton(in,at,setting.click)||at!=in.size())return false;setting.sendIncrement=increment!=0;setting.outputType=static_cast<OscValueType>(type);return !setting.displayName.isEmpty()&&setting.displayName.length()<=64&&inputEncoderSettingValid(setting);}

void defaultButton(ButtonInputSetting& b,const String& address,const String& seq){
  b=ButtonInputSetting();
  b.pressMessages[0].address=address;
  b.pressMessages[0].value="1.0";
  b.pressMessages[0].type=OSC_TYPE_FLOAT;
  b.releaseMessages[0].address=address;
  b.releaseMessages[0].value="0.0";
  b.releaseMessages[0].type=OSC_TYPE_FLOAT;
  b.sequence.address=seq; inputNormalizeSequence(b.sequence);
}
void setDefaultKey(uint8_t i){keySettings[i]=KeyInputSetting();keySettings[i].displayName=String("Key ")+(i+1);defaultButton(keySettings[i].button,String("/chainoscpad/key/")+(i+1),String("/chainoscpad/key/")+(i+1)+"/sequence");}
void setDefaultEncoder(){encoderSetting=EncoderInputSetting();defaultButton(encoderSetting.click,"/avatar/parameters/EncoderClick","/avatar/parameters/EncoderSeq");}
void setDefaults(){
  for(uint8_t i=0;i<KEY_COUNT;++i)setDefaultKey(i);
  setDefaultEncoder();
}
bool saveFile(const String& path,const Bytes& data){if(!fileSystemReady)return false;const String temporary=path+".tmp";LittleFS.remove(temporary);File file=LittleFS.open(temporary,FILE_WRITE);if(!file){Serial.printf("[LittleFS] open for write failed path=%s\n",temporary.c_str());return false;}const size_t written=file.write(data.data(),data.size());file.flush();file.close();if(written!=data.size()){Serial.printf("[LittleFS] short write path=%s bytes=%u written=%u\n",path.c_str(),static_cast<unsigned>(data.size()),static_cast<unsigned>(written));LittleFS.remove(temporary);return false;}if(!LittleFS.rename(temporary,path)){Serial.printf("[LittleFS] rename failed from=%s to=%s\n",temporary.c_str(),path.c_str());LittleFS.remove(temporary);return false;}Serial.printf("[LittleFS] saved path=%s bytes=%u used=%u total=%u\n",path.c_str(),static_cast<unsigned>(written),static_cast<unsigned>(LittleFS.usedBytes()),static_cast<unsigned>(LittleFS.totalBytes()));return true;}
bool loadFile(const String& path,Bytes& data){if(!fileSystemReady||!LittleFS.exists(path))return false;File file=LittleFS.open(path,FILE_READ);if(!file){Serial.printf("[LittleFS] open for read failed path=%s\n",path.c_str());return false;}const size_t length=file.size();if(length==0||length>65535){Serial.printf("[LittleFS] invalid file size path=%s bytes=%u\n",path.c_str(),static_cast<unsigned>(length));file.close();return false;}data.resize(length);const size_t read=file.read(data.data(),length);file.close();if(read!=length){Serial.printf("[LittleFS] short read path=%s bytes=%u read=%u\n",path.c_str(),static_cast<unsigned>(length),static_cast<unsigned>(read));return false;}return true;}
bool saveKeyFile(uint8_t i,const KeyInputSetting& setting){return saveFile(keyPath(i),encodeKey(setting));}
bool loadKeyFile(uint8_t i,KeyInputSetting& setting){Bytes data;if(!loadFile(keyPath(i),data))return false;const bool ok=decodeKey(data,setting);if(!ok)Serial.printf("[LittleFS] validation failed path=%s\n",keyPath(i).c_str());return ok;}
bool saveEncoderFile(const EncoderInputSetting& setting){return saveFile(ENCODER_PATH,encodeEncoder(setting));}
bool loadEncoderFile(EncoderInputSetting& setting){Bytes data;if(!loadFile(ENCODER_PATH,data))return false;const bool ok=decodeEncoder(data,setting);if(!ok)Serial.printf("[LittleFS] validation failed path=%s\n",ENCODER_PATH);return ok;}
}

bool inputParseFloat(const String& t,float& v){if(t.isEmpty()||t.length()>32)return false;errno=0;char* e=nullptr;v=strtof(t.c_str(),&e);return errno!=ERANGE&&e!=t.c_str()&&*e=='\0'&&isfinite(v);}
bool inputParseInt(const String& t,int32_t& v){if(t.isEmpty()||t.length()>16)return false;errno=0;char* e=nullptr;long x=strtol(t.c_str(),&e,10);if(errno==ERANGE||e==t.c_str()||*e!='\0'||x<INT32_MIN||x>INT32_MAX)return false;v=x;return true;}
bool inputOscAddressValid(const String& a){if(a.isEmpty()||a.length()>192||a[0]!='/')return false;for(size_t i=0;i<a.length();++i){char c=a[i];if(isWhitespace(c)||c=='#'||c=='*'||c==','||c=='?'||c=='['||c==']'||c=='{'||c=='}')return false;}return true;}
bool inputOscMessageValid(const OscMessageSetting& m){if(!inputOscAddressValid(m.address)||!typeValid(m.type)||m.value.length()>128)return false;if(m.type==OSC_TYPE_STRING)return true;if(m.type==OSC_TYPE_FLOAT){float v;return inputParseFloat(m.value,v);}int32_t v;return inputParseInt(m.value,v);}
void inputNormalizeSequence(SequenceSetting& s){if(!isfinite(s.start))s.start=0;if(!isfinite(s.end))s.end=10;if(!isfinite(s.step)||fabsf(s.step)<1e-9f)s.step=1;if(s.start<=s.end&&s.step<0)s.step=-s.step;if(s.start>s.end&&s.step>0)s.step=-s.step;s.current=s.start;}
bool inputButtonSettingValid(const ButtonInputSetting& b){if(b.mode>INPUT_MODE_SEQUENCE||b.pressMessageCount+b.releaseMessageCount>8||!inputOscAddressValid(b.sequence.address)||!typeValid(b.sequence.type)||!isfinite(b.sequence.start)||!isfinite(b.sequence.end)||!isfinite(b.sequence.step))return false;for(uint8_t i=0;i<b.pressMessageCount;++i)if(!inputOscMessageValid(b.pressMessages[i]))return false;for(uint8_t i=0;i<b.releaseMessageCount;++i)if(!inputOscMessageValid(b.releaseMessages[i]))return false;return true;}
bool inputEncoderSettingValid(const EncoderInputSetting& s){return inputOscAddressValid(s.rotationAddress)&&typeValid(s.outputType)&&isfinite(s.absoluteInputMin)&&isfinite(s.absoluteInputMax)&&s.absoluteInputMax>s.absoluteInputMin&&isfinite(s.incrementScale)&&isfinite(s.outputMin)&&isfinite(s.outputMax)&&inputButtonSettingValid(s.click);}
void inputSettingsSetup(){setDefaults();fileSystemReady=LittleFS.begin(true);if(!fileSystemReady){Serial.println("[LittleFS] mount failed; input settings use defaults");return;}Serial.printf("[LittleFS] mounted used=%u total=%u\n",static_cast<unsigned>(LittleFS.usedBytes()),static_cast<unsigned>(LittleFS.totalBytes()));for(uint8_t i=0;i<KEY_COUNT;++i){KeyInputSetting s=keySettings[i];if(loadKeyFile(i,s))keySettings[i]=s;}EncoderInputSetting e=encoderSetting;if(loadEncoderFile(e))encoderSetting=e;Serial.println("[Input settings] Ready");}
KeyInputSetting& inputKeySetting(uint8_t i){return keySettings[i<KEY_COUNT?i:0];}
EncoderInputSetting& inputEncoderSetting(){return encoderSetting;}
bool inputSettingsSaveKey(uint8_t i,const KeyInputSetting& s){if(i>=KEY_COUNT||s.displayName.isEmpty()||s.displayName.length()>64||!inputButtonSettingValid(s.button)){Serial.printf("[Input settings] Key %u validation failed\n",static_cast<unsigned>(i+1));return false;}if(!saveKeyFile(i,s)){Serial.printf("[Input settings] Key %u write failed\n",static_cast<unsigned>(i+1));return false;}KeyInputSetting v;if(!loadKeyFile(i,v)){Serial.printf("[Input settings] Key %u readback failed\n",static_cast<unsigned>(i+1));return false;}keySettings[i]=v;Serial.printf("[Input settings] Key %u saved\n",static_cast<unsigned>(i+1));return true;}
bool inputSettingsSaveEncoder(const EncoderInputSetting& s){if(s.displayName.isEmpty()||s.displayName.length()>64||!inputEncoderSettingValid(s)){Serial.println("[Input settings] Encoder validation failed");return false;}if(!saveEncoderFile(s)){Serial.println("[Input settings] Encoder write failed");return false;}EncoderInputSetting v;if(!loadEncoderFile(v)){Serial.println("[Input settings] Encoder readback failed");return false;}encoderSetting=v;Serial.println("[Input settings] Encoder saved");return true;}
bool inputSettingsResetKey(uint8_t i){if(i>=KEY_COUNT)return false;KeyInputSetting setting;setting.displayName=String("Key ")+(i+1);defaultButton(setting.button,String("/chainoscpad/key/")+(i+1),String("/chainoscpad/key/")+(i+1)+"/sequence");const bool ok=inputSettingsSaveKey(i,setting);Serial.printf("[Input settings] Key %u reset ok=%d\n",static_cast<unsigned>(i+1),ok);return ok;}
bool inputSettingsResetEncoder(){EncoderInputSetting setting;defaultButton(setting.click,"/avatar/parameters/EncoderClick","/avatar/parameters/EncoderSeq");const bool ok=inputSettingsSaveEncoder(setting);Serial.printf("[Input settings] Encoder reset ok=%d\n",ok);return ok;}
bool inputSettingsReset(){if(!fileSystemReady)return false;bool ok=true;for(uint8_t i=0;i<KEY_COUNT;++i){const String path=keyPath(i),temporary=path+".tmp";if(LittleFS.exists(path))ok=LittleFS.remove(path)&&ok;if(LittleFS.exists(temporary))ok=LittleFS.remove(temporary)&&ok;}const String encoderTemporary=String(ENCODER_PATH)+".tmp";if(LittleFS.exists(ENCODER_PATH))ok=LittleFS.remove(ENCODER_PATH)&&ok;if(LittleFS.exists(encoderTemporary))ok=LittleFS.remove(encoderTemporary)&&ok;setDefaults();Serial.printf("[LittleFS] input settings reset ok=%d used=%u total=%u\n",ok,static_cast<unsigned>(LittleFS.usedBytes()),static_cast<unsigned>(LittleFS.totalBytes()));return ok;}
