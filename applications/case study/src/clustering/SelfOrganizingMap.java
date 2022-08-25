package clustering;

import math.Matrix;
import math.Vector;

public class SelfOrganizingMap implements IClusteringAlgorithm {

	public static double defaultStartLearningRate = 0.1;
	
	protected int width;
	protected Vector nodes[];
	protected double startLearningRate;
	protected Matrix curData;
	
	public SelfOrganizingMap(int width) {
		this.width = width;
		this.startLearningRate = defaultStartLearningRate;
	}
	
	public SelfOrganizingMap(int width, double startLearningRate) {
		this.width = width;
		this.startLearningRate = startLearningRate;
	}

	public void feedData(Matrix data, int numClusters, double threshold,
			int maxIter) {
		curData = data;
		int dim = data.height();
		int num = data.width();

		int height = numClusters / width;
		double mapRadius = Math.max(width, height) / 2.0;
		double timeConstant = maxIter / Math.log(mapRadius);
		
		// initialize the model vector of each node (center) with small random
		// values
		nodes = new Vector[numClusters];
		for (int i = 0; i < numClusters; i++) {
			nodes[i] = new Vector(dim);
			for (int j = 0; j < dim; j++)
				nodes[i].set(j, Math.random());
		}
		
		for (int i = 0; i < maxIter; i++) {  // threshold is ignored
			// choose random vector from input data
			Vector v = data.getColumn((int) (Math.random() * (num - 1)));

			// find Best Matching Unit
			double minDist = Double.POSITIVE_INFINITY;
			int minDistIdx = 0;
			for (int j = 0; j < numClusters; j++) {
				double d = v.euclideanDistSqr(nodes[j]);
				if (d < minDist) {
					minDist = d;
					minDistIdx = j;
				}
			}

			// determine local neighborhood of BMU
			double r = mapRadius * Math.exp(-((double) i / timeConstant));
			r *= r;  // we use squared euclidean distance
			for (int j = 0; j < numClusters; j++) {
				int x = (minDistIdx % width) - (j % width);
				int y = (minDistIdx / width) - (j / width);
				double d = (x * x) + (y * y);

				if (d < r) {  // is within radius r
					// adjust model vector
					double learningRate = startLearningRate *
						Math.exp(-((double) i / maxIter));
					learningRate *= Math.exp(-d / (2.0 * r));
					nodes[j].add(Vector.mult(learningRate,
					                         Vector.sub(v, nodes[j])));
				}
			}
		}
	}

	public Matrix getAssociationMatrix() {
		if (curData == null)
			throw new RuntimeException();
		int num = curData.width();
		Matrix u = new Matrix(num, nodes.length);

		for (int i = 0; i < num; i++) {
			Vector v = curData.getColumn(i);

			// find closest center
			double minDist = Double.POSITIVE_INFINITY;
			int minDistIdx = 0;
			for (int j = 0; j < nodes.length; j++) {
				double d = v.euclideanDistSqr(nodes[j]);
				if (d < minDist) {
					minDist = d;
					minDistIdx = j;
				}
			}
			
			u.set(i, minDistIdx, 1.0);
		}

		return u;
	}

	public Vector[] getCenters() {
		return nodes;
	}
	
	public Matrix getUMatrix() {
		int height = nodes.length / width;
		Matrix u = new Matrix(height, width);
		
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int numNeighbors = 0;
				double sumDist = 0.0;
				
				int curNode = i * width + j;
				if (j > 0) {  // left
					sumDist += nodes[curNode].euclideanDist(
							nodes[i * width + (j - 1)]);
					numNeighbors++;
				}
				if (i > 0) {  // top
					sumDist += nodes[curNode].euclideanDist(
							nodes[(i - 1) * width + j]);
					numNeighbors++;
				}
				if (j < (width - 1)) {  // right
					sumDist += nodes[curNode].euclideanDist(
							nodes[i * width + (j + 1)]);
					numNeighbors++;
				}
				if (i < (height - 1)) {  // bottom
					sumDist += nodes[curNode].euclideanDist(
							nodes[(i + 1) * width + j]);
					numNeighbors++;
				}
				
				u.set(i, j, sumDist / numNeighbors);
			}
		}
		
		return u;
	}

}
