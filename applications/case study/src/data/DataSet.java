package data;

import math.Matrix;

public class DataSet {

	private Matrix data;
	private int row;
	
	public DataSet(Matrix data, int row) {
		this.data = data;
		this.row = row;
	}

	public double min() {
		double min = Double.MAX_VALUE;
		for (int i = 0; i < data.width(); i++) {
			double v = data.get(row, i);
			if (v < min)
				min = v;
		}
		return min;
	}
	
	public double max() {
		double max = Double.MIN_VALUE;
		for (int i = 0; i < data.width(); i++) {
			double v = data.get(row, i);
			if (v > max)
				max = v;
		}
		return max;		
	}
	
	public void normalize() {
		double min = min();
		double range = max() - min;
		for (int i = 0; i < data.width(); i++)
			data.set(row, i, (data.get(row, i) - min) / range);
	}
	
	public double mean() {
		int n = data.width();
		double mean = 0.0;
		for (int i = 0; i < n; i++)
			mean += data.get(row, i);
		mean /= n;
		return mean;
	}

	public double variance() {
		int n = data.width();
		double sum = 0.0;
		double mean = mean();
		
		for (int i = 0; i < n; i++) {
			double x = data.get(row, i) - mean;
			sum += x * x;
		}
		
		return (sum / (n - 1));
	}
	
}
