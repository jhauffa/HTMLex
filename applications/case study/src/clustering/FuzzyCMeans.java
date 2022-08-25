package clustering;

import math.Matrix;
import math.Vector;

public class FuzzyCMeans implements IClusteringAlgorithm {

	public static final double defaultM = 2.0;
	
	private double m;
	private Matrix u;
	private Vector[] centers;
	
	public FuzzyCMeans(double m) {
		this.m = m;
	}

	public void feedData(Matrix data, int numClusters, double maxNorm,
			int maxIter) {
		int numPoints = data.width();
		centers = new Vector[numClusters];

		u = new Matrix(numPoints, numClusters);
		double randomNumbers[] = new double[numClusters];
		for (int i = 0; i < numPoints; i++) {
			double sum = 0.0;
			for (int j = 0; j < numClusters; j++) {
				randomNumbers[j] = Math.random();
				sum += randomNumbers[j];
			}
			for (int j = 0; j < numClusters; j++)
				u.set(i, j, randomNumbers[j] / sum);
		}
		
		double objExp = 2.0 / (m - 1.0);
		
		int i = 0;
		double matrixNorm = Double.POSITIVE_INFINITY;
		while ((i < maxIter) && (matrixNorm > maxNorm)) {
			Matrix uPrev = new Matrix(u);
			
			// compute center vectors
			for (int j = 0; j < numClusters; j++) {
				centers[j] = new Vector(data.height());
				double div = 0.0;
				for (int k = 0; k < numPoints; k++) {
					double p = Math.pow(u.get(k, j), m);
					div += p;
					centers[j].add(Vector.mult(p, data.getColumn(k)));
				}
				centers[j].mult(1.0 / div);
			}
			
			// update u
			for (int j = 0; j < numPoints; j++) {
				Vector x = data.getColumn(j);
				for (int k = 0; k < numClusters; k++) {
					double obj = 0.0;
					for (int l = 0; l < numClusters; l++) {
						obj += Math.pow(x.euclideanDist(centers[k]) /
								        x.euclideanDist(centers[l]), objExp);
					}
					u.set(j, k, 1.0 / obj);
				}
			}
			
			Matrix diff = Matrix.sub(u, uPrev);
			matrixNorm = diff.frobeniusNorm();
		}
	}

	public Matrix getAssociationMatrix() {
		return u;
	}
	
	public Vector[] getCenters() {
		return centers;
	}
	
}
