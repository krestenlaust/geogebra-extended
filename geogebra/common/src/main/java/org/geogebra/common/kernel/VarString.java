package org.geogebra.common.kernel;

/**
 * Interface for all functions with named variables
 */
public interface VarString {

	/**
	 * Returns variable names separated by ", "
	 * 
	 * @param tpl
	 *            string template
	 * 
	 * @return variable names separated by ", "
	 */
	public String getVarString(StringTemplate tpl);
}
