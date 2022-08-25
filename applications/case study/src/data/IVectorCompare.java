package data;

import math.Vector;

public interface IVectorCompare {

	/**
	 * @return  <0 if v1 < v2
	 *           0 if v1 == v2
	 *          >0 if v1 > v2 
	 */
	public int compare(Vector v1, Vector v2);
	
}
