class CBaseEntity;

struct variant_hax
{
	const char * pszValue;
};

struct inputdata_t
{
	CBaseEntity *pActivator;// The entity that initially caused this chain of output events.
	CBaseEntity *pCaller;// The entity that fired this particular output.
	variant_hax value;// The data parameter for this output.
	int nOutputID;// The unique ID of the output that was fired.
};
