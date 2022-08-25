package clustering;

import math.Matrix;
import math.Vector;

public class KMeans implements IClusteringAlgorithm {

	private Vector centers[];
	private Matrix u;
	private boolean uIsNormalized;
	private int i;
	
	protected void initCenters(Vector centers[], Matrix data) {
		for (int i = 0; i < centers.length; i++) {
			int idx = (int) Math.round(Math.random() *
				(data.width() - 1));
			centers[i] = data.getColumn(idx);
		}
	}
	
	public void feedData(Matrix data, int numClusters, double threshold,
			int maxIter) {
		if (centers == null) {
			centers = new Vector[numClusters];
			initCenters(centers, data);
		}
		
		int numPoints = data.width();
		u = new Matrix(numPoints, numClusters);
		uIsNormalized = false;

		Vector newCenters[] = new Vector[numClusters];
		int numNewCenterMembers[] = new int[numClusters];

		i = 0;
		double maxCenterMovement;
		do {
			for (int j = 0; j < numClusters; j++)
				newCenters[j] = new Vector(data.height());
			java.util.Arrays.fill(numNewCenterMembers, 0);

			// associate every point with its closest center
			for (int j = 0; j < numPoints; j++) {
				Vector point = data.getColumn(j);
				
				// find closest center
				double minDist = Double.POSITIVE_INFINITY;
				int minDistCenter = 0;
				for (int k = 0; k < numClusters; k++) {
					double d = point.euclideanDistSqr(centers[k]);
					if (d < minDist) {
						minDist = d;
						minDistCenter = k;
					}
				}

				// update association matrix
				u.set(j, minDistCenter, i);
				
				// move center accordingly
				newCenters[minDistCenter].add(point);
				numNewCenterMembers[minDistCenter]++;
			}

			maxCenterMovement = 0.0;
			for (int j = 0; j < numClusters; j++) {
				if (numNewCenterMembers[j] > 0)
					newCenters[j].mult(1.0 / numNewCenterMembers[j]);
				else {
					// no associated points; randomly choose a new center
					int idx = (int) Math.round(Math.random() *
						(data.width() - 1));
					newCenters[j] = data.getColumn(idx);
				}
				double d = centers[j].euclideanDistSqr(newCenters[j]);
				if (d > maxCenterMovement)
					maxCenterMovement = d;
				centers[j] = newCenters[j];
			}
			i++;
		} while ((maxCenterMovement > threshold) && (i < maxIter));
	}

	public Matrix getAssociationMatrix() {
		if (!uIsNormalized) {
			// normalize association matrix: only values set in the last
			// iteration are relevant
			for (int j = 0; j < u.height(); j++) {
				for (int k = 0; k < u.width(); k++) {
					if (u.get(j, k) == (i - 1))
						u.set(j, k, 1.0);
					else
						u.set(j, k, 0.0);
				}
			}
			uIsNormalized = true;
		}

		return u;
	}
	
	public Vector[] getCenters() {
		return centers;
	}

}
