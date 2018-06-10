#include "Ob_Decimal.h"
#include "ttmathint.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ob_define.h"
#include "utility.h"
#include "ob_malloc.h"
using namespace std;
//add wenghaixing DECIMAL OceanBase_BankCommV0.1 2014_5_22:b
using namespace oceanbase::common;

ObDecimal::ObDecimal() :
		vscale_(0) {

	scale_ = 0;
//	word[0] = 0;
	word[0] = 0;
	precision_ = 38;

}

ObDecimal::~ObDecimal() {

//	ob_free(word);
}

ObDecimal::ObDecimal(const ObDecimal &other) {
	*this = other;
}

ObDecimal& ObDecimal::operator=(const ObDecimal &other) {
	if (OB_LIKELY(this != &other)) {
		this->vscale_ = other.vscale_;
		this->scale_ = other.scale_;
		this->precision_ = other.precision_;
		for (int8_t i = 0; i < NWORDS; ++i) {
			this->word[i] = other.word[i];
		}
	}
	return *this;
}

/*Name:round_to
 *Input:vscale,word
 *function:To be called by set_decimal(),input vscale and words,
 *         this function is to set ObDecimal's vscale& ttint
 *         to these two params;
 **/
int ObDecimal::round_to(uint32_t &vscale, TTInt &words) const {

//	OB_ASSERT(precision >= scale && 0 <= scale && NULL != words);
	int ret = OB_SUCCESS;
	ObDecimal clone = *this;
	vscale = clone.vscale_;
	words = clone.word[0];
	return ret;

}

/*Name:from
 *Input:precision,scale,vscale
 *function:this function is to be used in constructing ObDecimal from
 *         a ttint word & params.
 **/
void ObDecimal::from(uint32_t precision, uint32_t scale, uint32_t vscale,
		const TTInt words) {
//todo To add check before value decimal
	precision_ = precision;
	scale_ = scale;
	vscale_ = vscale;
	word[0] = words;

}

/*Name:from
 *Input:int64_t
 *function:give this function a integer,
 *         convert it into a decimal,and set in ObDecimal
 **/
int ObDecimal::from(int64_t i64) {

	//todo convert to long
	//	i64++;
	int ret = OB_SUCCESS;
	vscale_ = 0;
	TTInt BASE(10);
	TTInt p(precision_ - scale_);
	BASE.Pow(p);
	TTInt fi(i64);
	if (fi >= (BASE) || fi <= -(BASE)) {
		ret = OB_DECIMAL_UNLEGAL_ERROR;
		TBSYS_LOG(ERROR, "error when covert int to decimal!");
	} else
		word[0] = fi;
	return ret;
}

/*Name:from
 *Input:const char* str
 *function:call another from,to convert string into a decimal and
 *         set it into ObDecimal
 **/
int ObDecimal::from(const char* str) {

	return from(str, strlen(str));

}



/**Name:from
* input:buff,buflen
* function:to construct a decimal from a input buffer
* date:2014/6/12
**/
int ObDecimal::from(const char* buff, int64_t buf_len) {
    int ret = OB_SUCCESS;
	char int_buf[MAX_PRINTABLE_SIZE];
	char frac_buf[MAX_PRINTABLE_SIZE];
	memset(int_buf, 0, MAX_PRINTABLE_SIZE);
	memset(frac_buf, 0, MAX_PRINTABLE_SIZE);
	const char* s;
	int got_dot = 0;
	int got_digit = 0;
	int got_frac = 0;
	int got_num=0;
	int i = 0;
	int op = 0;
	int length = 0;
	bool is_valid=false;
    short int sign = 0;
	if(buff==NULL){
        ret=OB_ERR_UNEXPECTED;
        TBSYS_LOG(WARN, "failed to convert decimal from string buff,err=NULL buff ptr!");
	}
	if(buf_len>(int)MAX_PRINTABLE_SIZE)
    {
        ret=OB_ERR_UNEXPECTED;
        TBSYS_LOG(WARN,"failed to convert decimal from string buff,since buf_len is too long ");
    }
	if(OB_SUCCESS==ret){
        s = buff;
        sign = *s == '-' ? 1 : 0;   //sign mark
        if(buf_len==1&&!isdigit(*s)){
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN, "failed to convert char to decimal,invalid num=%c", *s);
        }
        else if(buf_len==2&&((*s == '-' || *s == '+')&&!isdigit(*(s+1)))){
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN, "failed to convert char to decimal,invalid num=%c", *s);
        };
        if(OB_SUCCESS==ret)
        {
            if (*s == '-' || *s == '+')
            {
                op = 1;   //sign count
                i++;
                ++s;
            }
            for (;; ++s)
            {
                if (i == buf_len)
                    break;
                if (isdigit(*s))
                {
                    if(*s!='0' || got_dot)
                        is_valid=true;  //zero before point and point don't count
                    if(is_valid)
                        got_digit++;   //valid num
                    got_num++;      //all num
                    if (got_dot)
                        got_frac++;
                }
                else
                {
                    if (!got_dot && *s == '.')
                        got_dot = 1;
                    else if (*s != '\0') {

                        ret = OB_ERR_UNEXPECTED;
                        TBSYS_LOG(ERROR, "failed to convert char to decimal,invalid num=%c", *s);
                    }
                }
                i++;

            }
        }
	}
    //modify xsl
    
    if(got_digit - got_frac > MAX_DECIMAL_DIGIT)     //max integer num
    {
        ret = OB_DECIMAL_UNLEGAL_ERROR;
        TBSYS_LOG(WARN, "decimal overflow!got_digit=%d,got_frac=%d", got_digit,got_frac);
    }
    int tmp_frac = got_frac;
    if(got_frac > MAX_DECIMAL_SCALE)    //37
    {
        got_digit = got_digit - (got_frac - MAX_DECIMAL_SCALE);
        got_frac = MAX_DECIMAL_SCALE;
    }
    if(got_digit > MAX_DECIMAL_DIGIT)
    {
        got_frac = got_frac -(got_digit - MAX_DECIMAL_DIGIT);
        got_digit = MAX_DECIMAL_DIGIT;
    }
    
    //modify e
    if(OB_SUCCESS==ret)
    {
        //int tmp_frac2 = 0;  //add xsl DECIMAL 2017_7
        length = got_num + got_dot + op;
        if (!got_dot) {    //no num after point
            //wwd added
            //    char buff1[] = ".0";
            memcpy(int_buf, buff, length);
            //    memcpy(int_buf+length,buff1,strlen(buff1));
            //    got_digit ++;
            //    got_frac = 1;
            //    got_dot = 1;
            //    got_num ++;
            //    length += 2;
            //    TBSYS_LOG(INFO,"add .0 to the end of the number!");
            TTInt whole;
            whole.FromString(int_buf);
            word[0] = whole;
        }
        else
        {
            int point_pos = length - tmp_frac - got_dot;  //modify xsl
            //int point_pos = length - got_frac - got_dot;
            memcpy(int_buf, buff, point_pos);
            TTInt whole;
            whole.FromString(int_buf);
            //modify xsl DECIMAL 2017_7
            /*
            tmp_frac2 = got_frac - 1;
            while(buff[point_pos + got_dot + tmp_frac2] == '0')
            {
                tmp_frac2--;
            }
            tmp_frac2++;
            TTInt p(tmp_frac2);
            */
            //modify e
            TTInt p(got_frac);
            TTInt BASE(10);
            BASE.Pow(p);
            whole = whole * BASE;
            //memcpy(frac_buf, buff + (point_pos + got_dot), tmp_frac2);
            memcpy(frac_buf, buff + (point_pos + got_dot), got_frac);
            //modify e
            TTInt part_float;
            part_float.FromString(frac_buf);

            if (sign)
                part_float.SetSign();
            whole += part_float;
            word[0] = whole;
        }
        /*
         *4.3 set it into word[0]
         */
        //vscale_ = tmp_frac2;     //add xsl DECIMAL
        vscale_ = got_frac;    //delete xsl DECIMAL
        scale_ = vscale_;      //all equal got_frac
        //precision_ = got_digit - got_frac + tmp_frac2; //add xsl DECIMAL
        precision_=got_digit;   //delete xsl DECIMAL
        if(precision_==scale_)precision_++;    //modify xsl 2017_4
	  }
	return ret;
//return ret;
}

/*Name:to_string
 *Input:char *buf,const int64_t buf_len
 *function:convert Obdecimal.word into a string buf
 *         for handling overflow:
 *         if num>kMAX,convert into 9999.9999,if num<kMin,convert into -9999.9999
 *         the main process is in function body
 **/
int64_t ObDecimal::to_string(char* buf, const int64_t buf_len) const {

    int pos = 0;
    int start = 0;
    int real_scale = 0;
    int tail_zero_count = 0;
    char e[1]={'\0'};
    char tmp[MAX_PRINTABLE_SIZE];
    memset(tmp, 0, MAX_PRINTABLE_SIZE);
    TTInt whole, float_part;
    TTInt BASE(10), p(vscale_);
    BASE.Pow(p);
    whole = word[0] / BASE;
    float_part = word[0] % BASE;
    if (precision_==0) {
        if (word[0].IsSign()) {
            tmp[pos] = '-';
            pos++;
        }

        strcpy(tmp + pos, _str_kMaxScaleFactor_38);
        pos += MAX_DECIMAL_DIGIT;

        strcpy(buf, tmp);

    }
    else{
      if (word[0].IsSign()) {
        tmp[pos] = '-';
        whole.ChangeSign();
        float_part.ChangeSign();
        start = 1;
      }

      string str_for_word;
      word[0].ToString(str_for_word);
      string str_for_int;
      whole.ToString(str_for_int);
      string str_for_frac;
      float_part.ToString(str_for_frac);

      int len_word = (int)str_for_word.length() - start;
      int len_float = (int) str_for_frac.length();
      pos = start;
      if (len_word < (int) vscale_) {
        whole = 0;
      }

      const char* int_str = str_for_int.c_str();
      strcpy(tmp + pos, int_str);
      pos += (int) strlen(int_str);
      if (vscale_ > scale_)
        real_scale = scale_;
      else {
        real_scale = vscale_;
        tail_zero_count = scale_ - vscale_;
      }
      if (scale_ != 0) {
        tmp[pos] = '.';
        pos++;
      }
      if (0 <= (int) vscale_ && scale_ != 0) {


         const char* float_str = str_for_frac.c_str();

//		if (len_word < (int) vscale_ && 0 < vscale_) {
//
//			for (int i = 0; i < (int) vscale_ - len_word; i++) {
//				tmp[pos] = '0';
//				++pos;
//			}
//			strcpy(tmp + pos, float_str);
//			pos += (int) strlen(float_str);
//		} else
            if (0 < vscale_) {
            for (int i = 0; i < (int) vscale_ - len_float; i++) {
                tmp[pos] = '0';
                ++pos;
            }
            strcpy(tmp + pos, float_str);
            pos += (int) strlen(float_str);
        }
        for (int i = 0; i < tail_zero_count; i++) {

            tmp[pos] = '0';
            pos++;
        }

        tmp[pos] = '\0';
        strncpy(buf, tmp, strlen(tmp) - (vscale_ - real_scale));
        pos = (int) strlen(tmp) - (vscale_ - real_scale);
    } else if (scale_ == 0) {
        if (len_word < (int) vscale_)
            strcpy(buf, tmp + start);
        else
            strcpy(buf, tmp);

    }
    }
    strcat(buf,e);
    OB_ASSERT(pos <= buf_len);
    return pos;
}

//add by kindaich 2014/4/18
int ObDecimal::to_int64(int64_t &i64) const {

//todo
//	if (!can_convert_to_int64()) {
//		TBSYS_LOG(WARN, "the number cannot be converted to int64, vscale=%hhd ",
//				vscale_);
//		ret = OB_VALUE_OUT_OF_RANGE;
//	}
//this function seem not to be used;
	i64++;
	return 0;
}
/*
  to convert a decimal to int64
*/
int ObDecimal::cast_to_int64(int64_t &i64) const {    //整数部分

    //todo
    int ret = OB_SUCCESS;
    //	TTInt kMaxScaleFactor(_str_kMaxScaleFactor);
    TTInt max_int("9223372036854775807");
    TTInt min_int("-9223372036854775808");
    TTInt Base(10),p(vscale_),whole(0);
    Base.Pow(p);
    whole=word[0]/Base;
    if(whole<min_int||whole>max_int){
        ret=OB_VALUE_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "failed to convert decimal to integer,err=overflowed!");
    }
    if(OB_SUCCESS==ret)
        i64=whole.ToInt();
    return ret;
}
//add xsl ECNU_DECIMAL 2017_2
int ObDecimal::cast_to_frac(int64_t &i64) const {

    //todo
    int ret = OB_SUCCESS;
    TTInt max_int("9223372036854775807");
    TTInt min_int("-9223372036854775808");
    TTInt Base(10),p(vscale_),whole(0);
    Base.Pow(p);
    whole=word[0]%Base;
    if(whole<min_int||whole>max_int){
        ret=OB_VALUE_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "failed to convert decimal to integer,err=overflowed!");
    }
    if(OB_SUCCESS==ret)
        i64=whole.ToInt();
    return ret;
}
//add e
/*
*to check if a decimal can covert into a int
*
int ObDecimal::can_convert_to_int64() const {
//	TTInt TiMax(MAX_INT);
//	TTInt TiMin(MIN_INT);
//	TTInt kMaxScaleFactor(_str_kMaxScaleFactor);
    int ret=OB_SUCCESS;
    TTInt max(9223372036854775807);
    TTInt min(-9223372036854775808);
    TTInt Base(10),p(vscale_),whole(0);
    Base.Pow(p);
    whole=whole*Base;
    if(whole<min||whole>max){
        ret=OB_VALUE_OUT_OF_RANGE;
        TBSYS_LOG(ERROR, "failed to convert decimal to integer,err=overflowed!");
    }
    else


	return ret ;
}
*/
std::ostream & oceanbase::common::operator<<(std::ostream &os,
		const ObDecimal& num) {
	char buff[MAX_PRINTABLE_SIZE];
	memset(buff, 0, MAX_PRINTABLE_SIZE);
	num.to_string(buff, MAX_PRINTABLE_SIZE);
	os << buff;
	return os;
}

void ObDecimal::reset() {
	set_scale(0);
	set_precision(0);
	set_vscale(0);
	memset(word, 0, sizeof(TTInt));

}
/*
  name:modify_value
  params:p:precision,s:scale
  function:before store data in oceanbase,we must modify value of decimal to fix scale_;
           eg:decimal(5,3) 3.1415->3.141

*/
int ObDecimal::modify_value(uint32_t p, uint32_t s) {    //modify vscale_
    UNUSED(p);
    int ret = OB_SUCCESS;
    int point_pos = 0;
    int len = 0;
    int is_neg = 0;
    char buf[MAX_PRINTABLE_SIZE];
    char out[MAX_PRINTABLE_SIZE];
    memset(buf, 0, MAX_PRINTABLE_SIZE);
    memset(out, 0, MAX_PRINTABLE_SIZE);
    //scale_=vscale_;
    to_string(buf, MAX_PRINTABLE_SIZE);
    if (buf[0] == '-')
        is_neg = 1;
    //确定整数部分位数
    while (buf[point_pos] != '\0')   //get point position
    {
        if (buf[point_pos] == '.')
            break;
        else
            point_pos++;          //num before point
    }
    /*
    int pre=is_neg;
    while(buf[pre] == '0')     //pre zero and -
    {
        pre++;
    }
    */
    if (point_pos - is_neg/*pre*/> (int) p - (int) s )
    {
        ret = OB_DECIMAL_UNLEGAL_ERROR;
        TBSYS_LOG(ERROR,"OB_DECIMAL_UNLEGAL_ERROR !,point_pos=%d,p= %d,s=%d buf=%s",
                  point_pos, p, s, buf);
        //return ret;
    }
    if (OB_SUCCESS==ret && s < vscale_)   //schema_s < decimal_s
    {
        if (0 == s)
        {
            len = point_pos;
            buf[len] = '\0';
            strcpy(out, buf);
        }
        else
        {
            len = s + point_pos + 1;
            if ((int) strlen(buf) < len)
            {
                ret = OB_ERR_UNEXPECTED;
                TBSYS_LOG(ERROR, "failed to assigned str buff=%s",buf);
            }
            else
            {
                buf[len++] = '\0';
                strncpy(out, buf, len);
            }
        }
        if(OB_SUCCESS != ret)
        {
            TBSYS_LOG(ERROR,"failed to modify value");
        }
        else if (OB_SUCCESS != (ret = this->from(out)))
        {
            TBSYS_LOG(ERROR, "failed convert str to decimal!");
        }
    }
    /*
    //add xsl ECNU_DECIMAL
    else if(ret ==OB_SUCCESS && s > vscale_ && vscale_ > 0)   //
    {
        if(point_pos < (int)precision_ && point_pos !=0)
        {
            len =point_pos + vscale_;
            while (buf[len] == '0')   //post 0
            {
                len--;          //
            }
            len = len + 1;
            if ((int) strlen(buf) < len)
            {
                ret = OB_ERR_UNEXPECTED;
                TBSYS_LOG(ERROR, "failed to assigned str buff=%s",buf);
            }
            else
            {
                buf[len++] = '\0';
                strncpy(out, buf, len);
            }
            if(OB_SUCCESS != ret)
            {
                TBSYS_LOG(ERROR,"failed to modify value");
            }
            else if (OB_SUCCESS != (ret = this->from(out)))
            {
                TBSYS_LOG(ERROR, "failed convert str to decimal!");
            }
        }
    }
    //add e
    */
    if(OB_SUCCESS==ret)
    {
        scale_ = s;
        //precision_ = point_pos - is_neg + s;   //add xsl ECNU_DECIMAL 2017_3
        precision_ = p;
    }
    return ret;
}

/**
 * add by herilyn chou
 *
 *we handle the result as DB2,the result is handled as follows:
 *ADD&SUB -----the precision_ of result is keeping the result's length
 *        -----the scale_ of result is the max of input
 *        -----the vscale_ of result is the max of input
 *MUL     -----the precision_ of result is keeping the result's length
 *        -----the scale_ of result is the sum of input's scale_
 *        -----the vscale_ of result is keeping the result's length
 *DIV	  -----the precision_ of result is DEFINE_MAX
 *        -----the scale_ of result is DEFINE_MAX
 *
 */
 //caculate add
int ObDecimal::add(const ObDecimal &other, ObDecimal &res) const {
    int ret = OB_SUCCESS;
	ObDecimal n1 = *this;
	ObDecimal n2 = other;
    int p = 0;
    TTLInt num1 = n1.get_words()[0];
    TTLInt num2 = n2.get_words()[0];
	TTInt BASE(10);
	if (n1.get_vscale() >= n2.get_vscale()) {
		p = (n1.get_vscale() - n2.get_vscale());
		BASE.Pow(p);
		num2 = num2 * BASE;
		res.vscale_ = n1.get_vscale();
	} else {
		p = (n2.get_vscale() - n1.get_vscale());
        BASE.Pow(p);
        num1 = num1 * BASE;
		res.vscale_ = n2.get_vscale();
	}
	if (num1.Add(num2) == 0) {
        string str_for_word;
        num1.ToString(str_for_word);
        int len_word=(int)strlen(str_for_word.c_str())-num1.IsSign();
        int len_int=len_word-res.vscale_;
        res.scale_=n1.scale_ >= n2.scale_? n1.scale_:n2.scale_;
        res.precision_=len_int+ res.scale_;
        if(res.precision_<=res.scale_)res.precision_=res.scale_+1;
		if (res.precision_ > (unsigned int) MAX_DECIMAL_DIGIT) {
			res.precision_ = (unsigned int) MAX_DECIMAL_DIGIT;
        }
		if (len_int > MAX_DECIMAL_DIGIT) {
             res.word[0]=num1.IsSign()?kMin:kMAX;
             res.precision_=0;
             ret = OB_VALUE_OUT_OF_RANGE;
		}
		if(ret==OB_SUCCESS&&len_word>MAX_DECIMAL_DIGIT){
            int len_modify=len_word-MAX_DECIMAL_DIGIT;
            BASE=10;
            BASE.Pow(len_modify);
            if(len_modify!=0)
            {
                res.word[0]=num1/BASE;
                res.vscale_=res.vscale_-len_modify;
                res.scale_=res.vscale_;
            }
        }
        else if(ret==OB_SUCCESS){
		   res.word[0] = num1;
		}
        if(ret!=OB_SUCCESS){
            char er1[MAX_PRINTABLE_SIZE];
            char er2[MAX_PRINTABLE_SIZE];
            char er3[MAX_PRINTABLE_SIZE];
            char er4[MAX_PRINTABLE_SIZE];
            char er5[MAX_PRINTABLE_SIZE];
            memset(er1,0,MAX_PRINTABLE_SIZE);
            memset(er2,0,MAX_PRINTABLE_SIZE);
            memset(er3,0,MAX_PRINTABLE_SIZE);
            memset(er4,0,MAX_PRINTABLE_SIZE);
            memset(er5,0,MAX_PRINTABLE_SIZE);
            n1.to_string(er1,MAX_PRINTABLE_SIZE);
            n2.to_string(er2,MAX_PRINTABLE_SIZE);
            strcpy(er3,n1.get_words()[0].ToString().c_str());
            strcpy(er4,n2.get_words()[0].ToString().c_str());
            strcpy(er5,num1.ToString().c_str());
            TBSYS_LOG(WARN,"faild to add/sub decimal,over flow,n1 =%s,ttint1=%s,n2=%s,ttint2=%s,res ttint=%s",er1,er3,er2,er4,er5);
        }
	}
	return ret;
}


int ObDecimal::sub(const ObDecimal &other, ObDecimal &res) const {
	ObDecimal n2 = other;
	n2.get_words()[0].ChangeSign();
    return add(n2,res);
}

int ObDecimal::mul(const ObDecimal &other, ObDecimal &res) const {
    int ret = OB_SUCCESS;
    ObDecimal n1 = *this;
    ObDecimal n2 = other;
    TTLInt num1 = n1.get_words()[0];
    TTLInt num2 = n2.get_words()[0];
    TTLInt base_to_modify(10);
    if (num1.Mul(num2) == 0) {
        int p=0;
        //v=n1.get_vscale()+n2.get_vscale();
        p=n1.get_precision()+n2.get_precision();
        if(p>MAX_DECIMAL_DIGIT)p=MAX_DECIMAL_DIGIT;
        string str_for_word;
        num1.ToString(str_for_word);
        int len_word=(int)strlen(str_for_word.c_str())-num1.IsSign();
        int len_int=len_word-n1.vscale_-n2.vscale_;
        //res.precision_=p;        //delete xsl ECNU_DECIMAL
        if ( len_int> MAX_DECIMAL_DIGIT) {
             res.word[0]=num1.IsSign()?kMin:kMAX;
             res.precision_=0;
             res.vscale_=n1.vscale_+n2.vscale_;
             res.scale_=n1.scale_+n2.scale_;
             ret = OB_VALUE_OUT_OF_RANGE;
        }

        if(ret==OB_SUCCESS)
        {
          if(len_word>MAX_DECIMAL_DIGIT)
          {
            res.vscale_=MAX_DECIMAL_DIGIT-len_int;
            int len_modify=n1.vscale_+n2.vscale_-res.vscale_;
            if(len_modify!=0)
            {
                base_to_modify.Pow(len_modify);
                res.word[0]=num1/base_to_modify;
            }
            else  //==0
                res.word[0]=num1;
           res.scale_=res.vscale_;
           res.precision_ = len_int + res.scale_; //add xsl ECNU_DECIMAL
         }
         else
          {
           res.word[0]=num1;
           res.vscale_=n1.vscale_+n2.vscale_;
           res.scale_=n1.scale_+n2.scale_;
           if(len_int+res.scale_>(int)MAX_DECIMAL_DIGIT)
           {
               res.scale_=MAX_DECIMAL_DIGIT-len_int;
           }
           res.precision_ = len_int + res.scale_; //add xsl ECNU_DECIMAL
           if(res.precision_ == res.scale_)res.precision_++;    //add xsl 2017_7
          }

        }
       else {
            char er1[MAX_PRINTABLE_SIZE];
            char er2[MAX_PRINTABLE_SIZE];
            char er3[MAX_PRINTABLE_SIZE];
            char er4[MAX_PRINTABLE_SIZE];
            char er5[MAX_PRINTABLE_SIZE];
            memset(er1,0,MAX_PRINTABLE_SIZE);
            memset(er2,0,MAX_PRINTABLE_SIZE);
            memset(er3,0,MAX_PRINTABLE_SIZE);
            memset(er4,0,MAX_PRINTABLE_SIZE);
            memset(er5,0,MAX_PRINTABLE_SIZE);
            n1.to_string(er1,MAX_PRINTABLE_SIZE);
            n2.to_string(er2,MAX_PRINTABLE_SIZE);
            strcpy(er3,n1.get_words()[0].ToString().c_str());
            strcpy(er4,n2.get_words()[0].ToString().c_str());
            strcpy(er5,num1.ToString().c_str());
            TBSYS_LOG(WARN,"ERROR to mul decimal,over flow,n1 =%s,ttint1=%s,n2=%s,ttint2=%s,res ttint=%s",er1,er3,er2,er4,er5);
        }
    }
    else
    {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "failed to mul decimal");
    }
    return ret;
}


int ObDecimal::div(const ObDecimal &other, ObDecimal &res) const {
	int ret = OB_SUCCESS;
	ObDecimal n1 = *this;
	ObDecimal n2 = other;
    int p = 0;
    TTLInt num1 = n1.get_words()[0];
    TTLInt num2 = n2.get_words()[0];

    TTLInt int_part;
	TTLInt BASE(10),base_to_modify(10);
	res.precision_=MAX_DECIMAL_DIGIT;


	if (n1.get_vscale() >= n2.get_vscale()) {
		p = (n1.get_vscale() - n2.get_vscale());
		BASE.Pow(p);
		num2 = num2 * BASE;

	} else {
		p = (n2.get_vscale() - n1.get_vscale());
		BASE.Pow(p);
		num1 = num1 * BASE;
	}
	num1 *= kMaxScaleFactor;
    if (num1.Div(num2) == 0) {     //OK
        int_part=num1/kMaxScaleFactor;
        if (int_part >= kMAX) {
            res.word[0] = kMAX;
            res.precision_=0;
            ret = OB_VALUE_OUT_OF_RANGE;
        } else if (int_part <= kMin) {
            res.word[0] = kMin;
            res.precision_=0;
            ret = OB_VALUE_OUT_OF_RANGE;
        }
        //整数部分长度
        if(ret==OB_SUCCESS){
            string str_for_int;
            int_part.ToString(str_for_int);
            int len_int=(int)strlen(str_for_int.c_str())-int_part.IsSign();
            res.vscale_=MAX_DECIMAL_DIGIT-len_int;
            if((int)res.vscale_>MAX_DECIMAL_SCALE){
                res.vscale_=MAX_DECIMAL_SCALE;
            }

            res.scale_=res.vscale_;
            int len_modify=MAX_DECIMAL_SCALE-res.vscale_;
            if(len_modify!=0){
                base_to_modify.Pow(len_modify);

                res.word[0]=num1/base_to_modify;
            }else
                res.word[0]=num1;
        }
        else{
            char er1[MAX_PRINTABLE_SIZE];
            char er2[MAX_PRINTABLE_SIZE];
            char er3[MAX_PRINTABLE_SIZE];
            char er4[MAX_PRINTABLE_SIZE];
            char er5[MAX_PRINTABLE_SIZE];
            memset(er1,0,MAX_PRINTABLE_SIZE);
            memset(er2,0,MAX_PRINTABLE_SIZE);
            memset(er3,0,MAX_PRINTABLE_SIZE);
            memset(er4,0,MAX_PRINTABLE_SIZE);
            memset(er5,0,MAX_PRINTABLE_SIZE);
            n1.to_string(er1,MAX_PRINTABLE_SIZE);
            n2.to_string(er2,MAX_PRINTABLE_SIZE);
            strcpy(er3,n1.get_words()[0].ToString().c_str());
            strcpy(er4,n2.get_words()[0].ToString().c_str());
            strcpy(er5,num1.ToString().c_str());
            TBSYS_LOG(WARN,"faild to mul decimal,over flow,n1 =%s,ttint1=%s,n2=%s,ttint2=%s,res ttint=%s",er1,er3,er2,er4,er5);
        }

    }
    else {
        ret = OB_ERROR;
		//TBSYS_LOG(ERROR, "failed to div decimal,num1=%s,num2=%s",n1.to_string().c_str(),n2.to_string().c_str());
	}

	return ret;
}

/*
  to compare two decimal
  use sub to check which is bigger
*/
int ObDecimal::compare(const ObDecimal &other) const {
	int ret = 0;
	ObDecimal res;
	if (OB_SUCCESS == this->sub(other, res)) {
		if (res.word[0].IsSign()) {
			ret = -1;
		} else if (!res.word[0].IsZero()) {
			ret = 1;
		}
	} else {
	}
	return ret;
}
/*
  to negate a decimal.
  eg:3.14-->-3.14
     -3.14-->3.14
*/
int ObDecimal::negate(ObDecimal &res) const {
	int ret = OB_SUCCESS;
	ObDecimal n1 = *this;
	res.precision_ = n1.precision_;
	res.scale_ = n1.scale_;
	res.vscale_ = n1.vscale_;
    TTInt num = n1.get_words()[0];

    num.ChangeSign();
    res.word[0] = num;
	return ret;
}

bool ObDecimal::operator<(const ObDecimal &other) const {
	int cmp = this->compare(other);
	return cmp < 0;
}
bool ObDecimal::operator<=(const ObDecimal &other) const {
	int cmp = this->compare(other);
	return cmp <= 0;
}
bool ObDecimal::operator>(const ObDecimal &other) const {
	int cmp = this->compare(other);
	return cmp > 0;
}
bool ObDecimal::operator>=(const ObDecimal &other) const {
	int cmp = this->compare(other);
	return cmp >= 0;
}
bool ObDecimal::operator==(const ObDecimal &other) const {
	int cmp = this->compare(other);
	return cmp == 0;
}
bool ObDecimal::operator!=(const ObDecimal &other) const {
	int cmp = this->compare(other);
	return cmp != 0;
}

//add :e
