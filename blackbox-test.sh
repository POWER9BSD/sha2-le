#!/bin/bash
bin_dir="./bin"
a="abc"
b="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL=JXEL"
c="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL="
d="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL=JXELFJWNXCJSD"

function cmp() {
    diff -u $1 _ref > _out
    if [[ $? -eq 0 ]]; then echo "$1 is Ok"; else echo "$1 is NOK!"; tail -n2 _out ; fi
    rm -f _out
}

for sha_bits in 256 512; do
  echo "Running tests for SHA-${sha_bits}:";
  for file in $a $b $c $d; do
    echo -n $file > _tmp
    ${bin_dir}/sha${sha_bits} _tmp > _c
    ${bin_dir}/sha${sha_bits}_ll_intrinsics _tmp > _intrinsics
    ${bin_dir}/sha${sha_bits}_ll_asm _tmp > _asm
    sha${sha_bits}sum _tmp | cut -d\  -f1 > _ref
    cmp _c
    cmp _intrinsics
    cmp _asm
    rm -f _ref _c _intrinsics _asm _tmp
  done
  echo ""
done

