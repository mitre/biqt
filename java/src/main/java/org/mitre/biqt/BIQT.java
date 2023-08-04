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
import java.util.ArrayList;
import java.io.IOException;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import cz.adamh.utils.NativeUtils;

/**
 * A new instance of BIQT which can be used to run available providers.
 */
public class BIQT {
  private static final Logger logger = LoggerFactory.getLogger(BIQT.class);
  private static final String os = System.getProperty("os.name");

  /* Loads the necessary native code libraries from the jar file */
  static {
    try {
      if (os.startsWith("Windows")) {
        NativeUtils.loadLibraryFromJar("/biqtapi.dll");
        NativeUtils.loadLibraryFromJar("/biqt_java.dll");
      } else if (os.startsWith("Mac")) {
        NativeUtils.loadLibraryFromJar("/libbiqtapi.dylib");
        NativeUtils.loadLibraryFromJar("/libbiqt_java.dylib");
      } else {
        NativeUtils.loadLibraryFromJar("/libbiqtapi.so");
        NativeUtils.loadLibraryFromJar("/libbiqt_java.so");
      }
    } catch (IOException e) {
      logger.warn("Second chance loading biqt libraries.");
      System.loadLibrary("biqtapi");
      System.loadLibrary("biqt_java");      
    }
  }

  /** Used in native code for storing a pointer to a BIQT instance */
  private long biqt_ptr;

  /**
   * Creates a new BIQT instance
   */
  public BIQT() { this.initialize(); }

  public native List<ProviderInfo> getProviders();

  /**
   * Initializes the necessary variables in native code
   */
  public native void initialize();

  /**
   * Runs one or more providers based on modality
   *
   * @param modality The modality of the provider to run, e.g. "iris"
   * @param inputFiles The files to analyze for quality with the specified
   *	providers.
   *
   *	@return A list of JSONObjects containing the provider results.
   */
  public List<JSONObject> runModality(String modality,
                                      List<String> inputFiles) {
    String result;
    JSONParser parser = new JSONParser();
    List<JSONObject> results = new ArrayList();
    for (String inputFile : inputFiles) {
      result = runModality(modality, inputFile);
      try {
        JSONObject json = (JSONObject)parser.parse(result);
        results.add(json);
      } catch (ParseException e) {
        logger.error(
            String.format("BIQT response for file '%s' was malformed: \n%s", inputFile, result),
            e);
      }
    }
    return results;
  }

  /**
   * Runs the specified provider
   *
   * @param provider The ProviderInfo related to the provider to run.
   * @param inputFiles The files to analyze for quality with the specified
   *	providers.
   *
   *	@return A list of JSONObjects containing the provider results.
   */
  public List<JSONObject> runProvider(ProviderInfo provider,
                                      List<String> inputFiles) {
    return this.runProvider(provider.getName(), inputFiles);
  }

  /**
   * Runs a provider with the specified name
   *
   * @param provider The name of the provider to run.
   * @param inputFiles The files to analyze for quality with the specified
   *	providers.
   *
   *	@return A list of JSONObjects containing the provider results.
   */
  public List<JSONObject> runProvider(String provider,
                                      List<String> inputFiles) {
    String result;
    JSONParser parser = new JSONParser();
    List<JSONObject> results = new ArrayList();

    for (String inputFile : inputFiles) {
      result = runProvider(provider, inputFile);
      try {
        JSONObject json = (JSONObject)parser.parse(result);
        results.add(json);
      } catch (ParseException e) {
        logger.error(
            String.format("BIQT response for file '%s' was malformed: \n%s", inputFile, result));
      }
    }
    return results;
  }

  /**
   * Calls the specified provider in native code.
   *
   * @param provider The name of the provider to be run.
   * @param inputFile The file to analyze with the given provider.
   */
  private native String runProvider(String provider, String inputFile);

  /**
   * Calls a provider with the specified modality in native code.
   *
   * @param provider The name of the provider to be run.
   * @param inputFile The file to analyze with the given provider.
   */
  private native String runModality(String modality, String inputfile);

  /**
   * Cleans up and frees any memory allocated in native code. This is
   * automatically called by the Java garbage collector.
   */
  protected void finalize() { this.cleanup(); }

  /**
   * Cleans up and frees any memory allocated in native code.
   */
  private native void cleanup();
}
