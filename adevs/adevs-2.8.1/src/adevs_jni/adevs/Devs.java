package adevs;

/**
 * This is the base class for all DEVS models.
 * @see Network
 * @see Atomic
 */
public abstract class Devs
{
	/**
	 * This is the Dynamic DEVS model transition function. For Atomic models,
	 * this method is called after each change of the model's state. If the
	 * method returns true, the model_transition method of the model's parent
	 * (a Network) is called. For a Network model, this method may be used
	 * to add and remove components the the model's set of components. Only models
	 * added and removed by the model_transition function are guaranteed to
	 * be properly handled by the Simulator. Method returns false by default.
	 */
	public boolean model_transition() { return false; }
	// This is set by the cpp Simulator to keep track of the C++ peer
	private long cpp_peer;
	// Used by the C++ simulator to typecheck models
	private boolean is_atomic;
	// Get the identification of the native peer. 
	public final long getNativePeer() { return cpp_peer; }
	// Constructor sets the flag for the model's type
	protected Devs(boolean is_atomic)
	{
		this.is_atomic = is_atomic;
		cpp_peer = 0;
	}
}
public abstract class dO {

/**
 * This is the Dynamic DEVS model transition function. For Atomic models,
 * this method is called after each change of the model's state. If the
 * method returns true, the model_transition method of the model's parent
 * (a Network) is called. For a Network model, this method may be used
 * to add and remove components the the model's set of components. Only models
 * added and removed by the model_transition function are guaranteed to
 * be properly handled by the Simulator. Method returns false by default.
 */
    public boolean model_transition() { return false; }
    // This is set by the cpp Simulator to keep track of the C++ peer
    private long cpp_peer;
    // Used by the C++ simulator to typecheck models
    private boolean is_atomic;
    // Get the identification of the native peer.
    public final long getNativePeer() { return cpp_peer; }
    // Constructor sets the flag for the model's type
    protected Devs(boolean is_atomic)
    {
        this.is_atomic = is_atomic;
        cpp_peer = 0;
    }

    public int MODELint() { return 1;}
    
    private void  excelGest() ;
    
    static models static static state simulator interface( int x , boolean y ){ return Simulator;  }
    
    abstract removed synchronized method { function = function + goto native long excelGest ; }
    
    
    this.is= is_atomic }
    
    
}


    public abstract class  PlanComptableBCM {
    
    private immobilisations;
    private  caisse ;
    private epargne ;
    
    public pretCL;
    
    PlanComptableBCM ( caisse=0 , epargne=0 , immobilisations = 1234) {
    
    
    
}