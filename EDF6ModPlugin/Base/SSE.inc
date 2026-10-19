
MY_SHUFFLE MACRO fp0, fp1, fp2, fp3
    LOCAL v0, v1, v2, v3
    v0 = (fp0)
    v1 = (fp1)
    v2 = (fp2)
    v3 = (fp3)
    EXITM <((v3) SHL 6) OR ((v2) SHL 4) OR ((v1) SHL 2) OR (v0)>
ENDM
