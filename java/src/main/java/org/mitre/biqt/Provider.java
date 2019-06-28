// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

package org.mitre.biqt;

import java.util.List;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

public abstract class Provider {
	private String providerName;
	private String message;
	private int errorCode;

	protected Provider() {
		this.providerName = this.getClass().getSimpleName();
	}

	/**
	 * Evaluates an image file and returns a JSON String
	 *
	 * @param filename The path to the file to be evaluated.
	 *
	 * @return A JSON String representation of the result
	 */
	public JSONObject evaluate(String filename) {
		JSONObject jsonResult = new JSONObject();
		JSONObject providerObject = new JSONObject();
		JSONObject resultObject = new JSONObject();
		JSONArray resultNameArray = new JSONArray();
		JSONObject features = new JSONObject();
		JSONObject metrics = new JSONObject();
		resultNameArray.add(resultObject);
		resultObject.put("features", features);
		resultObject.put("metrics", metrics);
		providerObject.put(this.providerName, resultObject);
		this.setResults(filename, features, metrics);
		jsonResult.put("errorCode", this.errorCode);
		jsonResult.put("message", this.message);
		jsonResult.put("provider", this.providerName);
		jsonResult.put("qualityResult", providerObject);

		return jsonResult;
	}

	protected abstract void setResults(String filename, JSONObject features,
									   JSONObject metrics);

	public static List<Class> getProviders() {
		return null;
	}
}
