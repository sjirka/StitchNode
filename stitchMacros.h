#define MCHECKERROR(STAT,MSG)       \
    if ( MS::kSuccess != STAT ) {   \
        cerr << MSG << endl;        \
            return MS::kFailure;    \
    }

#define MCHECKERRORNORET(STAT,MSG)  \
    if ( MS::kSuccess != STAT ) {   \
        cerr << MSG << endl;        \
    }


#define MAKE_TYPED_ATTR( NAME, LONGNAME, TYPE, DEFAULT, WRITABLE )				\
	MStatus NAME##_stat;														\
	MFnTypedAttribute NAME##_fn;												\
	NAME = NAME##_fn.create( LONGNAME, LONGNAME, TYPE, DEFAULT );				\
	NAME##_fn.setWritable(WRITABLE);											\
	NAME##_fn.setDisconnectBehavior(MFnAttribute::kReset);						\
	NAME##_stat = addAttribute( NAME );											\
	MCHECKERROR(NAME##_stat, "addAttribute error");

#define MAKE_NUMERIC_ATTR( NAME, LONGNAME, TYPE, DEFAULT, MIN, SOFTMAX)			\
	MStatus NAME##_stat;														\
	MFnNumericAttribute NAME##_fn;												\
	NAME = NAME##_fn.create( LONGNAME, LONGNAME, TYPE, DEFAULT );				\
	NAME##_fn.setMin(MIN);														\
	NAME##_fn.setSoftMax(SOFTMAX);												\
	NAME##_stat = addAttribute( NAME );											\
	MCHECKERROR(NAME##_stat, "addAttribute error");	

#define MAKE_CMP_NUMERIC_ATTR( NAME, LONGNAME, TYPE, DEFAULT, AFFECTS)					\
	MStatus NAME##_stat;														\
	MFnNumericAttribute NAME##_fn;												\
	NAME##X = NAME##_fn.create( LONGNAME"X", LONGNAME"X", TYPE, DEFAULT );		\
	NAME##Y = NAME##_fn.create( LONGNAME"Y", LONGNAME"Y", TYPE, DEFAULT );		\
	NAME##Z = NAME##_fn.create( LONGNAME"Z", LONGNAME"Z", TYPE, DEFAULT );		\
	NAME = NAME##_fn.create( LONGNAME, LONGNAME, NAME##X, NAME##Y, NAME##Z );	\
	NAME##_stat = addAttribute( NAME );											\
	MCHECKERROR(NAME##_stat, "addAttribute error");								\
	ATTRIBUTE_AFFECTS(NAME##X, AFFECTS);										\
	ATTRIBUTE_AFFECTS(NAME##Y, AFFECTS);										\
	ATTRIBUTE_AFFECTS(NAME##Z, AFFECTS);										

#define ADD_ATTRIBUTE( ATTR )                                               \
	MStatus ATTR##_stat;                                                    \
	ATTR##_stat = addAttribute( ATTR );                                     \
    MCHECKERROR( ATTR##_stat, "addAttribute: ATTR" )

#define ATTRIBUTE_AFFECTS( IN, OUT )                                        \
	MStatus IN##OUT##_stat;                                                 \
	IN##OUT##_stat = attributeAffects( IN, OUT );                           \
	MCHECKERROR(IN##OUT##_stat,"attributeAffects:" #IN "->" #OUT);

