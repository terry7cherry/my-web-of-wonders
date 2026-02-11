export default [
    `
    <tr>
      <td><label for="session-rounds-played">Rounds played: </label></td>
      <td>
        <input type="text"
               id="session-rounds-played"
               class="session-data"
               name="session-rounds-played" />
      </td>
    </tr>
    `,
    `
    <tr>
      <td><label for="session-total-bet">Total bet: </label></td>
      <td>
        <input type="text"
               id="session-total-bet"
               class="session-data currency feeds-rtp"
               name="session-total-bet" />
      </td>
    </tr>
    `,
    `
    <tr>
      <td><label for="session-total-won">Total won: </label></td>
      <td>
        <input type="text"
               id="session-total-won"
               class="session-data currency"
               name="session-total-won" />
      </td>
    </tr>
    `,
    `
    <tr>
      <td><label for="session-rtp">RTP: </label></td>
      <td>
        <input type="text"
               id="session-rtp"
               class="session-data percent calculated numerator-total-won divisor-total-bet"
               name="session-rtp" />
      </td>
    </tr>
    `,
    `
    <tr>
      <td><label for="session-freespins-sessions">Freespins Sessions: </label></td>
      <td>
        <input type="text"
               id="session-freespins-sessions"
               class="session-data"
               name="session-freespins-sessions" />
      </td>
    </tr>
    `,
    `
    <tr>
      <td><label for="session-freespins-played">Total freespins: </label></td>
      <td>
        <input type="text"
               id="session-freespins-played"
               class="session-data"
               name="session-freespins-played" />
      </td>
    </tr>
    `
];
