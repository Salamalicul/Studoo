// ============================================================================
//  StudooBilling: compras dentro de la app con Google Play (Billing Library 8).
//    - "studoo_pro": pago único que desbloquea Studoo Pro
//    - "donacion_pequena", "donacion_mediana", "donacion_grande": donaciones
//      (se pueden hacer las veces que quieras)
//  Solo se usa en la versión de Google Play. El código C++ (src/Pro.cpp) llama
//  a estas funciones y pregunta el estado cada poco tiempo.
// ============================================================================
package com.studoo.app;

import android.app.Activity;
import android.content.Context;
import android.util.Base64;
import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public final class StudooBilling
{
    static final String PRO = "studoo_pro";
    static final String[] DONATIONS = { "donacion_pequena", "donacion_mediana", "donacion_grande" };

    private static BillingClient client;
    private static String publicKey = "";
    private static volatile boolean ready = false;
    private static volatile int pro = -1;            // -1 = todavía no se sabe, 0 = no, 1 = sí
    private static final Map<String, ProductDetails> details = new ConcurrentHashMap<>();
    private static final Map<String, String> prices = new ConcurrentHashMap<>();
    // Avisos para C++: "pro_comprado", "donacion_ok", "pendiente", "cancelado", "error:..."
    private static final ConcurrentLinkedQueue<String> events = new ConcurrentLinkedQueue<>();

    // --- Llamadas desde C++ ----------------------------------------------------
    public static synchronized void start (Context context, String key)
    {
        publicKey = key != null && ! key.isEmpty() ? key : StudooPlayKey.KEY;
        if (client != null)
            return;

        client = BillingClient.newBuilder (context.getApplicationContext())
                     .setListener ((result, purchases) -> onPurchasesUpdated (result, purchases))
                     .enablePendingPurchases (PendingPurchasesParams.newBuilder().enableOneTimeProducts().build())
                     .enableAutoServiceReconnection()
                     .build();
        connect();
    }

    public static boolean isReady()      { return ready; }
    public static int     proState()     { return pro; }
    public static String  price (String productId) { String p = prices.get (productId); return p != null ? p : ""; }
    public static String  nextEvent()    { String e = events.poll(); return e != null ? e : ""; }

    public static void refresh()
    {
        if (ready)
        {
            queryPurchases();
            queryProducts();
        }
        else if (client != null)
        {
            connect();
        }
    }

    public static boolean buy (Activity activity, String productId)
    {
        ProductDetails d = details.get (productId);
        if (! ready || d == null || activity == null)
        {
            events.add ("error:La tienda de Google Play no está disponible ahora mismo. Revisa tu conexión e inténtalo otra vez.");
            refresh();
            return false;
        }
        List<BillingFlowParams.ProductDetailsParams> list = new ArrayList<>();
        list.add (BillingFlowParams.ProductDetailsParams.newBuilder().setProductDetails (d).build());
        BillingResult r = client.launchBillingFlow (activity, BillingFlowParams.newBuilder().setProductDetailsParamsList (list).build());
        if (r.getResponseCode() != BillingClient.BillingResponseCode.OK)
        {
            events.add ("error:No se pudo abrir el pago (" + r.getResponseCode() + ").");
            return false;
        }
        return true;
    }

    // --- Funcionamiento interno -------------------------------------------------
    private static void connect()
    {
        client.startConnection (new BillingClientStateListener()
        {
            @Override public void onBillingSetupFinished (BillingResult result)
            {
                ready = result.getResponseCode() == BillingClient.BillingResponseCode.OK;
                if (ready)
                {
                    queryProducts();
                    queryPurchases();
                }
            }

            @Override public void onBillingServiceDisconnected() { ready = false; }
        });
    }

    private static void queryProducts()
    {
        List<QueryProductDetailsParams.Product> products = new ArrayList<>();
        products.add (QueryProductDetailsParams.Product.newBuilder().setProductId (PRO).setProductType (BillingClient.ProductType.INAPP).build());
        for (String id : DONATIONS)
            products.add (QueryProductDetailsParams.Product.newBuilder().setProductId (id).setProductType (BillingClient.ProductType.INAPP).build());

        client.queryProductDetailsAsync (QueryProductDetailsParams.newBuilder().setProductList (products).build(), (result, queryResult) ->
        {
            if (result.getResponseCode() != BillingClient.BillingResponseCode.OK || queryResult == null)
                return;
            for (ProductDetails d : queryResult.getProductDetailsList())
            {
                details.put (d.getProductId(), d);
                ProductDetails.OneTimePurchaseOfferDetails offer = d.getOneTimePurchaseOfferDetails();
                if (offer != null)
                    prices.put (d.getProductId(), offer.getFormattedPrice());
            }
        });
    }

    private static void queryPurchases()
    {
        client.queryPurchasesAsync (QueryPurchasesParams.newBuilder().setProductType (BillingClient.ProductType.INAPP).build(), (result, purchases) ->
        {
            if (result.getResponseCode() != BillingClient.BillingResponseCode.OK)
                return;
            boolean hasPro = false;
            for (Purchase p : purchases)
            {
                if (p.getPurchaseState() != Purchase.PurchaseState.PURCHASED || ! isGenuine (p))
                    continue;
                if (p.getProducts().contains (PRO))
                {
                    hasPro = true;
                    acknowledge (p);
                }
                else
                {
                    consume (p, false); // una donación que no se terminó de procesar
                }
            }
            pro = hasPro ? 1 : 0;
        });
    }

    private static void onPurchasesUpdated (BillingResult result, List<Purchase> purchases)
    {
        final int code = result.getResponseCode();
        if (code == BillingClient.BillingResponseCode.USER_CANCELED)
        {
            events.add ("cancelado");
            return;
        }
        if (code == BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED)
        {
            queryPurchases();
            events.add ("pro_comprado");
            return;
        }
        if (code != BillingClient.BillingResponseCode.OK || purchases == null)
        {
            events.add ("error:El pago no se ha completado (" + code + ").");
            return;
        }

        for (Purchase p : purchases)
        {
            if (p.getPurchaseState() == Purchase.PurchaseState.PENDING)
            {
                events.add ("pendiente");
                continue;
            }
            if (p.getPurchaseState() != Purchase.PurchaseState.PURCHASED || ! isGenuine (p))
                continue;

            if (p.getProducts().contains (PRO))
            {
                pro = 1;
                acknowledge (p);
                events.add ("pro_comprado");
            }
            else
            {
                consume (p, true);
            }
        }
    }

    // Google exige "confirmar" las compras en 3 días o las devuelve
    private static void acknowledge (Purchase p)
    {
        if (p.isAcknowledged())
            return;
        client.acknowledgePurchase (AcknowledgePurchaseParams.newBuilder().setPurchaseToken (p.getPurchaseToken()).build(), r -> {});
    }

    // Las donaciones se "gastan" para poder volver a donar
    private static void consume (Purchase p, boolean announce)
    {
        client.consumeAsync (ConsumeParams.newBuilder().setPurchaseToken (p.getPurchaseToken()).build(), (r, token) ->
        {
            if (announce && r.getResponseCode() == BillingClient.BillingResponseCode.OK)
                events.add ("donacion_ok");
        });
    }

    // Comprueba la firma de Google con la clave pública de la app (Play Console >
    // Monetizar > Configuración de monetización > Licencias). Sin clave, no se comprueba.
    private static boolean isGenuine (Purchase p)
    {
        if (publicKey.isEmpty())
            return true;
        try
        {
            PublicKey key = KeyFactory.getInstance ("RSA").generatePublic (new X509EncodedKeySpec (Base64.decode (publicKey, Base64.DEFAULT)));
            Signature sig = Signature.getInstance ("SHA1withRSA");
            sig.initVerify (key);
            sig.update (p.getOriginalJson().getBytes ("UTF-8"));
            return sig.verify (Base64.decode (p.getSignature(), Base64.DEFAULT));
        }
        catch (Exception e)
        {
            return false;
        }
    }
}
