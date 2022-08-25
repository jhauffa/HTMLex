package clustering;

import math.Matrix;
import math.Vector;

public interface IClusteringAlgorithm {

	public void feedData(Matrix data, int numClusters, double threshold,
		int maxIter);

	public Matrix getAssociationMatrix();
	public Vector[] getCenters(); 

}
