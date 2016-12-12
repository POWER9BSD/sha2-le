#!/bin/bash

a="abc"
b="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL=JXEL"
c="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL="
d="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL=JXELFJWNXCJSD"

for sha_bits in 256 512; do
  echo "Running tests for SHA-${sha_bits}:";
  for file in $a $b $c $d; do
    echo -n $file > _tmp
    ./sha${sha_bits} _tmp > _my
    sha${sha_bits}sum _tmp | cut -d\  -f1 > _ref
    diff -u _my _ref > _tmp # overwrite, I know... but it's useless now
    if [[ $? -eq 0 ]]; then echo 'Ok'; else echo 'NOK!'; tail -n2 _tmp ; fi
  done
  echo ""
done
rm _ref _my _tmp
