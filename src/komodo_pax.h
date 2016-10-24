

int32_t NUM_PRICES; uint32_t *PVALS;

#define USD 0

#define MAX_CURRENCIES 32
char CURRENCIES[][8] = { "USD", "EUR", "JPY", "GBP", "AUD", "CAD", "CHF", "NZD", // major currencies
    "CNY", "RUB", "MXN", "BRL", "INR", "HKD", "TRY", "ZAR", "PLN", "NOK", "SEK", "DKK", "CZK", "HUF", "ILS", "KRW", "MYR", "PHP", "RON", "SGD", "THB", "BGN", "IDR", "HRK",
    "KMD" };

uint64_t M1SUPPLY[] = { 3317900000000, 6991604000000, 667780000000000, 1616854000000, 331000000000, 861909000000, 584629000000, 46530000000, // major currencies
    45434000000000, 16827000000, 3473357229000, 306435000000, 27139000000000, 2150641000000, 347724099000, 1469583000000, 749543000000, 1826110000000, 2400434000000, 1123925000000, 3125276000000, 13975000000000, 317657000000, 759706000000000, 354902000000, 2797061000000, 162189000000, 163745000000, 1712000000000, 39093000000, 1135490000000000, 80317000000,
    100000000 };

#define MIND 1000
uint32_t MINDENOMS[] = { MIND, MIND, 100*MIND, MIND, MIND, MIND, MIND, MIND, // major currencies
    10*MIND, 100*MIND, 10*MIND, MIND, 100*MIND, 10*MIND, MIND, 10*MIND, MIND, 10*MIND, 10*MIND, 10*MIND, 10*MIND, 100*MIND, MIND, 1000*MIND, MIND, 10*MIND, MIND, MIND, 10*MIND, MIND, 10000*MIND, 10*MIND, // end of currencies
10*MIND,
};

uint64_t komodo_paxvol(uint64_t volume,uint64_t price)
{
    if ( volume < 10000000000 )
        return((volume * price) / 1000000000);
    else if ( volume < (uint64_t)10 * 10000000000 )
        return((volume * (price / 10)) / 100000000);
    else if ( volume < (uint64_t)100 * 10000000000 )
        return(((volume / 10) * (price / 10)) / 10000000);
    else if ( volume < (uint64_t)1000 * 10000000000 )
        return(((volume / 10) * (price / 100)) / 1000000);
    else if ( volume < (uint64_t)10000 * 10000000000 )
        return(((volume / 100) * (price / 100)) / 100000);
    else if ( volume < (uint64_t)100000 * 10000000000 )
        return(((volume / 100) * (price / 1000)) / 10000);
    else if ( volume < (uint64_t)1000000 * 10000000000 )
        return(((volume / 1000) * (price / 1000)) / 1000);
    else if ( volume < (uint64_t)10000000 * 10000000000 )
        return(((volume / 1000) * (price / 10000)) / 100);
    else return(((volume / 10000) * (price / 10000)) / 10);
}

void pax_rank(uint64_t *ranked,uint32_t *pvals)
{
    int32_t i; uint64_t vals[32],sum = 0;
    for (i=0; i<32; i++)
    {
        vals[i] = komodo_paxvol(M1SUPPLY[i] / MINDENOMS[i],pvals[i]);
        sum += vals[i];
    }
    for (i=0; i<32; i++)
    {
        ranked[i] = (vals[i] * 1000000000) / sum;
        printf("%.6f ",(double)ranked[i]/1000000000.);
    }
};

int32_t dpow_readprices(uint8_t *data,uint32_t *timestampp,double *KMDBTCp,double *BTCUSDp,double *CNYUSDp,uint32_t *pvals)
{
    uint32_t kmdbtc,btcusd,cnyusd; int32_t i,n,len = 0;
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)timestampp);
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&n);
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&kmdbtc); // /= 1000
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&btcusd); // *= 1000
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&cnyusd);
    *KMDBTCp = ((double)kmdbtc / (1000000000. * 1000.));
    *BTCUSDp = ((double)btcusd / (1000000000. / 1000.));
    *CNYUSDp = ((double)cnyusd / 1000000000.);
    for (i=0; i<n-3; i++)
    {
        len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&pvals[i]);
        //printf("%u ",pvals[i]);
    }
    pvals[i++] = kmdbtc;
    pvals[i++] = btcusd;
    pvals[i++] = cnyusd;
    //printf("OP_RETURN prices\n");
    return(n);
}

/*uint32_t PAX_val32(double val)
 {
 uint32_t val32 = 0; struct price_resolution price;
 if ( (price.Pval= val*1000000000) != 0 )
 {
 if ( price.Pval > 0xffffffff )
 printf("Pval overflow error %lld\n",(long long)price.Pval);
 else val32 = (uint32_t)price.Pval;
 }
 return(val32);
 }*/

double PAX_val(uint32_t pval,int32_t baseid)
{
    //printf("PAX_val baseid.%d pval.%u\n",baseid,pval);
    if ( baseid >= 0 && baseid < MAX_CURRENCIES )
        return(((double)pval / 1000000000.) / MINDENOMS[baseid]);
    return(0.);
}

void komodo_pvals(int32_t height,uint32_t *pvals,uint8_t numpvals)
{
    int32_t i,nonz; double KMDBTC,BTCUSD,CNYUSD; uint32_t kmdbtc,btcusd,cnyusd;
    if ( numpvals >= 35 )
    {
        for (nonz=i=0; i<32; i++)
        {
            if ( pvals[i] != 0 )
                nonz++;
            //printf("%u ",pvals[i]);
        }
        if ( nonz == 32 )
        {
            kmdbtc = pvals[i++];
            btcusd = pvals[i++];
            cnyusd = pvals[i++];
            KMDBTC = ((double)kmdbtc / (1000000000. * 1000.));
            BTCUSD = ((double)btcusd / (1000000000. / 1000.));
            CNYUSD = ((double)cnyusd / 1000000000.);
            PVALS = (uint32_t *)realloc(PVALS,(NUM_PRICES+1) * sizeof(*PVALS) * 36);
            PVALS[36 * NUM_PRICES] = height;
            memcpy(&PVALS[36 * NUM_PRICES + 1],pvals,sizeof(*pvals) * 35);
            NUM_PRICES++;
            //printf("OP_RETURN.%d KMD %.8f BTC %.6f CNY %.6f NUM_PRICES.%d\n",height,KMDBTC,BTCUSD,CNYUSD,NUM_PRICES);
        }
    }
}

int32_t komodo_baseid(char *origbase)
{
    int32_t i; char base[64];
    for (i=0; origbase[i]!=0&&i<sizeof(base); i++)
        base[i] = toupper((int32_t)(origbase[i] & 0xff));
    base[i] = 0;
    for (i=0; i<=MAX_CURRENCIES; i++)
        if ( strcmp(CURRENCIES[i],base) == 0 )
            return(i);
    printf("illegal base.(%s) %s\n",origbase,base);
    return(-1);
}

uint64_t komodo_paxcalc(uint32_t *pvals,int32_t baseid,int32_t relid,uint64_t volume)
{
    uint32_t pvalb,pvalr,kmdbtc,btcusd; uint64_t baseusd,kmdusd,sum,ranked[32]; int32_t i;
    if ( (pvalb= pvals[baseid]) != 0 )
    {
        if ( relid == MAX_CURRENCIES )
        {
            kmdbtc = pvals[MAX_CURRENCIES];
            btcusd = pvals[MAX_CURRENCIES + 1];
            if ( pvals[USD] != 0 && kmdbtc != 0 && btcusd != 0 )
            {
                baseusd = ((uint64_t)pvalb * 1000000000) / pvals[USD];
                kmdusd = ((uint64_t)kmdbtc * 1000000000) / btcusd;
                //printf("base -> USD %llu, BTC %llu KMDUSD %llu\n",(long long)baseusd,(long long)btcusd,(long long)kmdusd);
                return(volume * ((baseusd * 1000000000) / kmdusd));
            }
        }
        else if ( baseid == relid )
        {
            pax_rank(ranked,pvals);

        }
        else if ( (pvalr= pvals[relid]) != 0 )
        {
            baserel = ((uint64_t)pvalb * 1000000000) / pvalr;
            return(komodo_paxvol(volume,baserel));
        }
    }
}

uint64_t komodo_paxprice(int32_t height,char *base,char *rel,uint64_t volume)
{
    int32_t baseid=-1,relid=-1,i,ht; uint32_t kmdbtc,btcusd,pvalb,pvalr,*ptr; uint64_t baserel,baseusd,kmdusd;
    if ( (baseid= komodo_baseid(base)) >= 0 && (relid= komodo_baseid(rel)) >= 0 )
    {
        for (i=NUM_PRICES-1; i>=0; i--)
        {
            ptr = &PVALS[36 * i];
            if ( *ptr < height )
                return(komodo_paxcalc(&ptr[1],baseid,relid,volume));
        }
    } else printf("paxprice invalid base.%s %d, rel.%s %d\n",base,baseid,rel,relid);
    return(0);
}

int32_t komodo_paxprices(uint32_t *timestamps,uint64_t *prices,int32_t max,int32_t width,char *base,char *rel)
{
    int32_t baseid=-1,relid=-1,i,ht; uint32_t kmdbtc,btcusd,pvalb,pvalr,*ptr; uint64_t baserel,baseusd,kmdusd;
    if ( (baseid= komodo_baseid(base)) >= 0 && (relid= komodo_baseid(rel)) >= 0 )
    {
        for (i=NUM_PRICES-1; i>=0; i--)
        {
            ptr = &PVALS[36 * i];
        }
    }
}

int32_t komodo_pax_opreturn(uint8_t *opret,int32_t maxsize)
{
    static uint32_t lastcrc;
    FILE *fp; char fname[512]; uint32_t crc32,check,timestamp; int32_t i,n,retval,fsize,len=0; uint8_t data[8192];
#ifdef WIN32
    sprintf(fname,"%s\\%s",GetDataDir(false).string().c_str(),(char *)"komodofeed");
#else
    sprintf(fname,"%s/%s",GetDataDir(false).string().c_str(),(char *)"komodofeed");
#endif
    if ( (fp= fopen(fname,"rb")) != 0 )
    {
        fseek(fp,0,SEEK_END);
        fsize = (int32_t)ftell(fp);
        rewind(fp);
        if ( fsize <= maxsize-4 && fsize <= sizeof(data) && fsize > sizeof(crc32) )
        {
            if ( (retval= (int32_t)fread(data,1,fsize,fp)) == fsize )
            {
                len = iguana_rwnum(0,data,sizeof(crc32),(void *)&crc32);
                check = calc_crc32(0,data+sizeof(crc32),(int32_t)(fsize-sizeof(crc32)));
                if ( check == crc32 )
                {
                    double KMDBTC,BTCUSD,CNYUSD; uint32_t pvals[128];
                    dpow_readprices(&data[len],&timestamp,&KMDBTC,&BTCUSD,&CNYUSD,pvals);
                    if ( 0 && lastcrc != crc32 )
                    {
                        for (i=0; i<32; i++)
                            printf("%u ",pvals[i]);
                        printf("t%u n.%d KMD %f BTC %f CNY %f (%f)\n",timestamp,n,KMDBTC,BTCUSD,CNYUSD,CNYUSD!=0?1./CNYUSD:0);
                    }
                    if ( timestamp > time(NULL)-600 )
                    {
                        n = komodo_opreturnscript(opret,'P',data+sizeof(crc32),(int32_t)(fsize-sizeof(crc32)));
                        if ( 0 && lastcrc != crc32 )
                        {
                            for (i=0; i<n; i++)
                                printf("%02x",opret[i]);
                            printf(" coinbase opret[%d] crc32.%u:%u\n",n,crc32,check);
                        }
                    } else printf("t%u too old for %u\n",timestamp,(uint32_t)time(NULL));
                    lastcrc = crc32;
                } else printf("crc32 %u mismatch %u\n",crc32,check);
            } else printf("fread.%d error != fsize.%d\n",retval,fsize);
        } else printf("fsize.%d > maxsize.%d or data[%d]\n",fsize,maxsize,(int32_t)sizeof(data));
        fclose(fp);
    }
    return(n);
}
